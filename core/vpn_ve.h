/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * Copyright (C) 2007, 2008
 *      National Institute of Information and Communications Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CORE_VPN_VE_H
#define _CORE_VPN_VE_H

#define	CRYPT_SEND_PACKET_LIFETIME		((UINT64)1000)
#define	CRYPT_MAX_LOG_QUEUE_LINES		32

typedef struct VPN_NIC VPN_NIC;
typedef struct VPN_CTX VPN_CTX;

#endif

// Secure VM Project
// ve (Virtual Ethernet)
// 
// By dnobori@cs.tsukuba.ac.jp

#ifndef	VE_COMMON_H
#define	VE_COMMON_H

// ��
#ifdef	_MSC_VER
#ifdef	_WIN64
typedef	unsigned long long int		intptr;
#else	// _WIN64
typedef	unsigned int				intptr;
#endif	// _WIN64
#else	// _MSC_VER
typedef unsigned long int			intptr;
#endif	// _MSC_VER

// ���
#define	VE_BUFSIZE					1600		// 1 ��� call ������������Хåե�������
#define	VE_MAX_PACKET_SIZE			1514		// ����ѥ��åȥ�����

// ��¤��
typedef struct VE_CTL VE_CTL;

struct VE_CTL
{
	unsigned int RetValue;			// ����� (1: ����, 0: ����)
	unsigned int Operation;			// ��������
	unsigned int EthernetType;		// Ethernet �ǥХ����μ���
	unsigned int NumQueue;			// �ѥ��åȤ��Ϥ�¦�Υ��塼�˻ĤäƤ���ѥ��åȤο�
	unsigned int PacketSize;		// �ѥ��åȥ�����
	unsigned char PacketData[VE_MAX_PACKET_SIZE];	// �ѥ��åȥǡ���
	unsigned char _padding[66];		// padding
};

#define	VE_OP_GET_NEXT_SEND_PACKET		0	// �����������٤��ѥ��åȤμ��� (vpn -> vmm -> guest)
#define	VE_OP_PUT_RECV_PACKET			1	// ���������ѥ��åȤν񤭹��� (guest -> vmm -> vpn)
#define	VE_OP_GET_LOG					2	// ���� 1 �Լ��� (vmm -> guest)

#define	VE_TYPE_PHYSICAL				0	// ʪ��Ū�� LAN ������
#define	VE_TYPE_VIRTUAL					1	// ����Ū�� LAN ������



#endif	// VE_COMMON_H


