#ifndef E1000_H
#define E1000_H 1

#include <KiSimple.h>
#include <PCI/pci.h>
#include <PMM/pmm.h>
#include <VMM/vmm.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
	uint32_t Bar0;
	uint8_t MAC[6];
	uint8_t* MMIO;
	uint32_t* TxDesc;
	uint8_t* TxBuffer;
	uint32_t* RxDesc;
	uint8_t* RxBuffer;
	int TxHead;
	int TxTail;
	int RxHead;
	int RxTail;
} E1000_t;

bool E1000_Init(PciDevice_t dev);
bool E1000_Send(const uint8_t* data, size_t len);
bool E1000_Receive(uint8_t* buf, size_t* len);
bool TcpIpSend(uint32_t src_ip, uint16_t src_port,
               uint32_t dst_ip, uint16_t dst_port,
               const uint8_t* data, size_t len);
bool TcpIpRecv(uint8_t* out_data, size_t* out_len,
               uint32_t* src_ip, uint16_t* src_port,
               uint32_t* dst_ip, uint16_t* dst_port);
bool UDPReceive(uint8_t* buf, size_t* len,
                uint32_t* src_ip, uint16_t* src_port,
                uint32_t* dst_ip, uint16_t* dst_port);
bool UDPSend(uint32_t src_ip, uint16_t src_port,
             uint32_t dst_ip, uint16_t dst_port,
             const uint8_t* data, size_t len);
bool DHCPGetIp(uint32_t* out_ip);
bool ResolveDNS(const char* hostname, uint32_t dns_server_ip, uint32_t src_ip, uint32_t* out_ip);

#endif /* E1000_H */
