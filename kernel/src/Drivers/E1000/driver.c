#include "../E1000.h"

static E1000_t e;

#define REG(x) (*(volatile uint32_t*)(e.MMIO + (x)))
#define E1000_REG_CTRL     0x0000
#define E1000_REG_STATUS   0x0008
#define E1000_REG_EEC      0x0010
#define E1000_REG_IMS      0x00D0
#define E1000_REG_RCTL     0x0100
#define E1000_REG_TCTL     0x0400
#define E1000_REG_RDBAL    0x2800
#define E1000_REG_RDLEN    0x2808
#define E1000_REG_RDH      0x2810
#define E1000_REG_RDT      0x2818
#define E1000_REG_TDBAL    0x3800
#define E1000_REG_TDLEN    0x3808
#define E1000_REG_TDH      0x3810
#define E1000_REG_TDT      0x3818
#define E1000_REG_RA       0x5400
#define E1000_REG_ICR      0x00C0

#define NUM_RX 32
#define NUM_TX 8
#define DESC_ALIGN 16

typedef struct {
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
} __attribute__((packed)) TxDesc;

typedef struct {
	uint64_t addr;
	uint16_t length;
	uint16_t csum;
	uint8_t status;
	uint8_t errors;
	uint16_t special;
} __attribute__((packed)) RxDesc;

#define DMA_REGION_BASE 0xC0000000
#define DMA_REGION_SIZE (16 * 4096) // 16 pages = 64 KiB
static uint8_t dma_bitmap[16] = {0}; // one byte per page

void* alloc_dma_contig(size_t size) {
	size_t npages = (size + 4095) / 4096;

	// Only support up to 16 pages for now
	for (size_t i = 0; i <= 16 - npages; i++) {
		bool free = true;
		for (size_t j = 0; j < npages; j++) {
			if (dma_bitmap[i + j]) {
				free = false;
				break;
			}
		}
		if (!free) continue;

		// Mark as used
		for (size_t j = 0; j < npages; j++) dma_bitmap[i + j] = 1;

		// Allocate and map pages
		for (size_t j = 0; j < npages; j++) {
			void* phys = KiPmmAlloc();
			if (!phys) return NULL;
			void* virt = (void*)(DMA_REGION_BASE + (i + j) * 4096);
			KiMMap(virt, phys, MMAP_PRESENT | MMAP_RW);
		}

		return (void*)(DMA_REGION_BASE + i * 4096);
	}

	return NULL;
}

bool E1000_Init(PciDevice_t dev) {
	if (dev.vendor_id != 0x8086) return false;
	if (!(dev.device_id == 0x100E || dev.device_id == 0x10D3)) return false;

	e.MMIO = (uint8_t*)dev.MMIOBase;
	if (!e.MMIO) return false;

	REG(E1000_REG_CTRL) |= (1 << 6);

	int timeout = 100000;
	while (((REG(E1000_REG_EEC) & (1 << 9)) == 0) && timeout--);
	if (timeout <= 0) return false;

	uint32_t ra0 = REG(E1000_REG_RA + 0);
	uint32_t ra1 = REG(E1000_REG_RA + 4);
	for (int i = 0; i < 4; i++) e.MAC[i] = (ra0 >> (i * 8)) & 0xFF;
	for (int i = 0; i < 2; i++) e.MAC[i + 4] = (ra1 >> (i * 8)) & 0xFF;

	e.RxBuffer = alloc_dma_contig(NUM_RX * 2048);
	e.RxDesc = (uint32_t*)alloc_dma_contig(sizeof(RxDesc) * NUM_RX);
	for (int i = 0; i < NUM_RX; i++) {
		RxDesc* d = ((RxDesc*)e.RxDesc) + i;
		d->addr = (uint64_t)(uintptr_t)(e.RxBuffer + i * 2048);
		d->status = 0;
	}
	REG(E1000_REG_RDBAL) = (uint32_t)(uintptr_t)e.RxDesc;
	REG(E1000_REG_RDLEN) = NUM_RX * sizeof(RxDesc);
	REG(E1000_REG_RDH) = 0;
	REG(E1000_REG_RDT) = NUM_RX - 1;
	REG(E1000_REG_RCTL) = (1 << 1) | (1 << 15) | (1 << 26);

	e.TxBuffer = alloc_dma_contig(NUM_TX * 2048);
	e.TxDesc = (uint32_t*)alloc_dma_contig(sizeof(TxDesc) * NUM_TX);
	for (int i = 0; i < NUM_TX; i++) {
		TxDesc* d = ((TxDesc*)e.TxDesc) + i;
		d->addr = (uint64_t)(uintptr_t)(e.TxBuffer + i * 2048);
		d->status = 1;
	}
	REG(E1000_REG_TDBAL) = (uint32_t)(uintptr_t)e.TxDesc;
	REG(E1000_REG_TDLEN) = NUM_TX * sizeof(TxDesc);
	REG(E1000_REG_TDH) = 0;
	REG(E1000_REG_TDT) = 0;
	REG(E1000_REG_TCTL) = (1 << 1) | (1 << 3) | (1 << 5);

	return true;
}

bool E1000_Send(const uint8_t* data, size_t len) {
	int t = e.TxTail;
	TxDesc* d = ((TxDesc*)e.TxDesc) + t;
	if (!(d->status & 0x1)) return false;

	memcpy(e.TxBuffer + t * 2048, data, len);
	d->length = len;
	d->cmd = (1 << 0) | (1 << 3);
	d->status = 0;

	e.TxTail = (t + 1) % NUM_TX;
	REG(E1000_REG_TDT) = e.TxTail;
	return true;
}

bool E1000_Receive(uint8_t* buf, size_t* len) {
	int r = e.RxTail;
	RxDesc* d = ((RxDesc*)e.RxDesc) + r;
	if (!(d->status & 0x1)) return false;

	*len = d->length;
	memcpy(buf, e.RxBuffer + r * 2048, *len);
	d->status = 0;

	e.RxTail = (r + 1) % NUM_RX;
	REG(E1000_REG_RDT) = e.RxTail;
	return true;
}

bool TcpIpSend(uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port, const uint8_t* data, size_t len) {
	uint8_t packet[1500];
	uint8_t* eth = packet;
	uint8_t* ip  = eth + 14;
	uint8_t* tcp = ip + 20;
	uint8_t* payload = tcp + 20;

	memset(packet, 0, sizeof(packet));

	eth[0] = 0xFF; eth[1] = 0xFF; eth[2] = 0xFF;
	eth[3] = 0xFF; eth[4] = 0xFF; eth[5] = 0xFF;
	memcpy(&eth[6], e.MAC, 6);
	eth[12] = 0x08;
	eth[13] = 0x00;

	ip[0] = 0x45;
	ip[1] = 0x00;
	uint16_t total_len = 20 + 20 + len;
	ip[2] = total_len >> 8;
	ip[3] = total_len & 0xFF;
	ip[4] = 0; ip[5] = 0;
	ip[6] = 0x40;
	ip[7] = 0x00;
	ip[8] = 64;
	ip[9] = 6;

	ip[12] = (src_ip >> 24) & 0xFF;
	ip[13] = (src_ip >> 16) & 0xFF;
	ip[14] = (src_ip >> 8) & 0xFF;
	ip[15] = src_ip & 0xFF;
	ip[16] = (dst_ip >> 24) & 0xFF;
	ip[17] = (dst_ip >> 16) & 0xFF;
	ip[18] = (dst_ip >> 8) & 0xFF;
	ip[19] = dst_ip & 0xFF;

	uint32_t csum = 0;
	for (int i = 0; i < 20; i += 2)
		csum += (ip[i] << 8) + ip[i + 1];
	while (csum >> 16)
		csum = (csum & 0xFFFF) + (csum >> 16);
	csum = ~csum;
	ip[10] = (csum >> 8) & 0xFF;
	ip[11] = csum & 0xFF;

	tcp[0] = src_port >> 8;
	tcp[1] = src_port & 0xFF;
	tcp[2] = dst_port >> 8;
	tcp[3] = dst_port & 0xFF;
	tcp[4] = 0; tcp[5] = 0; tcp[6] = 0; tcp[7] = 0;
	tcp[8] = 0; tcp[9] = 0; tcp[10] = 0; tcp[11] = 0;
	tcp[12] = 0x50;
	tcp[13] = 0x18;
	tcp[14] = 0xFF;
	tcp[15] = 0xFF;
	tcp[16] = 0;
	tcp[17] = 0;

	memcpy(payload, data, len);
	return E1000_Send(packet, 14 + total_len);
}

bool TcpIpRecv(uint8_t* out_data, size_t* out_len, uint32_t* src_ip, uint16_t* src_port, uint32_t* dst_ip, uint16_t* dst_port) {
	uint8_t packet[1600];
	size_t plen = 0;
	if (!E1000_Receive(packet, &plen)) return false;

	if (plen < 54) return false;

	uint8_t* eth = packet;
	if (eth[12] != 0x08 || eth[13] != 0x00) return false;

	uint8_t* ip = eth + 14;
	if (ip[9] != 6) return false;

	uint8_t* tcp = ip + 20;
	uint8_t* payload = tcp + 20;

	uint16_t ip_total_len = (ip[2] << 8) | ip[3];
	size_t tcp_len = ip_total_len - 20 - 20;
	if (tcp_len > *out_len) tcp_len = *out_len;

	*src_ip = (ip[12] << 24) | (ip[13] << 16) | (ip[14] << 8) | ip[15];
	*dst_ip = (ip[16] << 24) | (ip[17] << 16) | (ip[18] << 8) | ip[19];
	*src_port = (tcp[0] << 8) | tcp[1];
	*dst_port = (tcp[2] << 8) | tcp[3];
	*out_len = tcp_len;

	memcpy(out_data, payload, tcp_len);
	return true;
}

bool DHCPGetIp(uint32_t* out_ip) {
	uint8_t packet[548] = {0};

	packet[0] = 0x01;
	packet[1] = 0x01;
	packet[2] = 0x06;
	packet[3] = 0x00;

	uint32_t xid = 0x12345678;
	packet[4] = (xid >> 24) & 0xFF;
	packet[5] = (xid >> 16) & 0xFF;
	packet[6] = (xid >> 8) & 0xFF;
	packet[7] = xid & 0xFF;

	packet[8] = 0x00;
	packet[9] = 0x00;
	packet[10] = 0x80;
	packet[11] = 0x00;

	packet[28] = e.MAC[0];
	packet[29] = e.MAC[1];
	packet[30] = e.MAC[2];
	packet[31] = e.MAC[3];
	packet[32] = e.MAC[4];
	packet[33] = e.MAC[5];

	packet[236] = 0x63;
	packet[237] = 0x82;
	packet[238] = 0x53;
	packet[239] = 0x63;

	packet[240] = 53;
	packet[241] = 1;
	packet[242] = 1;

	packet[243] = 55;
	packet[244] = 2;
	packet[245] = 1;
	packet[246] = 3;

	packet[247] = 255;

	if (!UDPSend(0, 68, 0xFFFFFFFF, 67, packet, 548)) return false;

	uint8_t recv_buf[548];
	uint32_t src_ip;
	uint16_t src_port, dst_port;
	size_t recv_len = sizeof(recv_buf);

	for (int i = 0; i < 1000; i++) {
		if (!UDPReceive(recv_buf, &recv_len, &src_ip, &src_port, out_ip, &dst_port)) continue;

		if (recv_len >= 248 && recv_buf[240] == 0x63 && recv_buf[241] == 0x82 &&
		    recv_buf[242] == 0x53 && recv_buf[243] == 0x63) {
			uint8_t* options = &recv_buf[240];
			for (size_t j = 4; j < recv_len - 4;) {
				uint8_t type = options[j++];
				if (type == 255) break;
				uint8_t len = options[j++];
				if (type == 53 && options[j] == 2) {
					*out_ip = (recv_buf[16] << 24) | (recv_buf[17] << 16) |
					          (recv_buf[18] << 8) | recv_buf[19];
					return true;
				}
				j += len;
			}
		}
	}
	return false;
}

bool UDPReceive(uint8_t* buf, size_t* len, uint32_t* src_ip, uint16_t* src_port,
                uint32_t* dst_ip, uint16_t* dst_port) {
	uint8_t packet[1600];
	size_t plen = 0;
	if (!E1000_Receive(packet, &plen)) return false;

	if (plen < 42) return false;

	uint8_t* eth = packet;
	if (eth[12] != 0x08 || eth[13] != 0x00) return false;

	uint8_t* ip = eth + 14;
	uint8_t ip_header_len = (ip[0] & 0x0F) * 4;
	if (ip[9] != 17) return false;

	uint16_t ip_total_len = (ip[2] << 8) | ip[3];
	if (ip_total_len < ip_header_len + 8) return false;

	uint8_t* udp = ip + ip_header_len;
	uint16_t udp_len = (udp[4] << 8) | udp[5];
	if (plen < 14 + ip_header_len + udp_len) return false;

	*src_ip = (ip[12] << 24) | (ip[13] << 16) | (ip[14] << 8) | ip[15];
	*dst_ip = (ip[16] << 24) | (ip[17] << 16) | (ip[18] << 8) | ip[19];
	*src_port = (udp[0] << 8) | udp[1];
	*dst_port = (udp[2] << 8) | udp[3];

	uint8_t* udp_payload = udp + 8;
	size_t udp_payload_len = udp_len - 8;
	if (udp_payload_len > *len) udp_payload_len = *len;

	memcpy(buf, udp_payload, udp_payload_len);
	*len = udp_payload_len;
	return true;
}

bool UDPSend(uint32_t src_ip, uint16_t src_port,
             uint32_t dst_ip, uint16_t dst_port,
             const uint8_t* data, size_t len) {
	uint8_t packet[1500];
	uint8_t* eth = packet;
	uint8_t* ip  = eth + 14;
	uint8_t* udp = ip + 20;
	uint8_t* payload = udp + 8;

	memset(packet, 0, sizeof(packet));

	// Ethernet header
	eth[0] = 0xFF; eth[1] = 0xFF; eth[2] = 0xFF;
	eth[3] = 0xFF; eth[4] = 0xFF; eth[5] = 0xFF;
	memcpy(&eth[6], e.MAC, 6);
	eth[12] = 0x08;
	eth[13] = 0x00;

	// IP header
	ip[0] = 0x45;
	ip[1] = 0x00;
	uint16_t total_len = 20 + 8 + len;
	ip[2] = total_len >> 8;
	ip[3] = total_len & 0xFF;
	ip[4] = 0; ip[5] = 0;
	ip[6] = 0x40;
	ip[7] = 0x00;
	ip[8] = 64;
	ip[9] = 17; // UDP

	ip[12] = (src_ip >> 24) & 0xFF;
	ip[13] = (src_ip >> 16) & 0xFF;
	ip[14] = (src_ip >> 8) & 0xFF;
	ip[15] = src_ip & 0xFF;
	ip[16] = (dst_ip >> 24) & 0xFF;
	ip[17] = (dst_ip >> 16) & 0xFF;
	ip[18] = (dst_ip >> 8) & 0xFF;
	ip[19] = dst_ip & 0xFF;

	// IP checksum
	uint32_t csum = 0;
	for (int i = 0; i < 20; i += 2)
		csum += (ip[i] << 8) + ip[i + 1];
	while (csum >> 16)
		csum = (csum & 0xFFFF) + (csum >> 16);
	csum = ~csum;
	ip[10] = (csum >> 8) & 0xFF;
	ip[11] = csum & 0xFF;

	// UDP header
	udp[0] = src_port >> 8;
	udp[1] = src_port & 0xFF;
	udp[2] = dst_port >> 8;
	udp[3] = dst_port & 0xFF;
	uint16_t udp_len = 8 + len;
	udp[4] = udp_len >> 8;
	udp[5] = udp_len & 0xFF;
	udp[6] = 0;
	udp[7] = 0;

	// Payload
	memcpy(payload, data, len);

	return E1000_Send(packet, 14 + total_len);
}

static uint16_t dns_transaction_id = 0x1234;

bool ResolveDNS(const char* hostname, uint32_t dns_server_ip, uint32_t src_ip, uint32_t* out_ip) {
	uint8_t query[512];
	memset(query, 0, sizeof(query));

	query[0] = (dns_transaction_id >> 8) & 0xFF;
	query[1] = dns_transaction_id & 0xFF;
	query[2] = 0x01;
	query[5] = 0x01;

	size_t offset = 12;
	const char* h = hostname;
	while (*h) {
		const char* label_start = h;
		while (*h && *h != '.') h++;
		uint8_t len = h - label_start;
		query[offset++] = len;
		memcpy(&query[offset], label_start, len);
		offset += len;
		if (*h == '.') h++;
	}
	query[offset++] = 0x00;
	query[offset++] = 0x00;
	query[offset++] = 0x01;
	query[offset++] = 0x00;
	query[offset++] = 0x01;

	if (!UDPSend(src_ip, 12345, dns_server_ip, 53, query, offset)) return false;

	for (int tries = 0; tries < 1000; tries++) {
		uint8_t reply[512];
		size_t rlen = sizeof(reply);
		uint32_t rip;
		uint16_t sport, dport;
		if (!UDPReceive(reply, &rlen, &rip, &sport, &src_ip, &dport)) continue;
		if (sport != 53 || rlen < 12 || (reply[0] != query[0] || reply[1] != query[1])) continue;
		uint16_t answer_count = (reply[6] << 8) | reply[7];
		size_t ptr = offset;

		while (answer_count-- && ptr < rlen) {
			while (ptr < rlen && reply[ptr] != 0) {
				if ((reply[ptr] & 0xC0) == 0xC0) {
					ptr += 2;
					break;
				}
				ptr += reply[ptr] + 1;
			}
			if (ptr + 10 > rlen) break;
			ptr += 1;
			uint16_t type = (reply[ptr] << 8) | reply[ptr + 1];
			ptr += 8;
			uint16_t rdlength = (reply[ptr] << 8) | reply[ptr + 1];
			ptr += 2;
			if (type == 1 && rdlength == 4 && ptr + 4 <= rlen) {
				*out_ip = (reply[ptr] << 24) | (reply[ptr + 1] << 16) |
				          (reply[ptr + 2] << 8) | reply[ptr + 3];
				return true;
			}
			ptr += rdlength;
		}
	}
	return false;
}
