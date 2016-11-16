/*
 * Copyright (c) 2003-2013  Simon Ekstrand
 * Copyright (c) 2010-2015  Joachim Nilsson
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>

#include "netcalc.h"


void show_split_networks_v4(struct if_info *ifi, uint32_t splitmask, int v4args, struct misc_args m_argv4)
{
	uint32_t diff, start, end;
	size_t maxlen = 0;

	if (splitmask < ifi->v4ad.n_nmask) {
		warnx("Cannot subnet to /%d with this base network, use a prefix len > /%d",
		      numtolen(splitmask), numtolen(ifi->v4ad.n_nmask));
		return;
	}

	printf("\n[Split network/%d]\n", numtolen(splitmask));

	diff  = 0xffffffff - splitmask + 1;
	start = ifi->v4ad.n_naddr;
	end   = ifi->v4ad.n_naddr + diff - 1;

	/* Figure out max width of a network string. */
	while (1) {
		char buf[30];
		size_t len;

		len = snprintf(buf, sizeof(buf), "%s", numtoquad(start));
		if (len > maxlen)
			maxlen = len;

		start += diff;
		if (end == 0xffffffff || end >= ifi->v4ad.n_broadcast)
			break;
		end += diff;
	}

	start = ifi->v4ad.n_naddr;
	end = ifi->v4ad.n_naddr + diff - 1;
	while (1) {
		char buf[30];

		snprintf(buf, sizeof(buf), "%s", numtoquad(start));
		printf("Network  : \e[34m%-*s - %-15s\e[0m  ", (int)maxlen, buf, numtoquad(end));
		printf("Netmask  : \e[34m%s\e[0m\n", numtoquad(splitmask));

		start += diff;
		if (end == 0xffffffff || end >= ifi->v4ad.n_broadcast)
			break;
		end += diff;
	}
}

void print_cf_info_v4(struct if_info *ifi)
{
	uint32_t num, bcast, len, min, max;
	char temp[21];

	num = ifi->v4ad.n_broadcast - ifi->v4ad.n_naddr - 1;
	len = ifi->v4ad.n_nmaskbits;
	if (len > 30) {
		if (len == 31) {
			num = 2;
			min = ifi->v4ad.n_naddr;
			max = min + 1;
		} else
			num = 1;
	} else {
		min = ifi->v4ad.n_naddr + 1;
		max = ifi->v4ad.n_broadcast - 1;
	}

	printf("Address  : \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(ifi->v4ad.n_haddr), numtobitmap(ifi->v4ad.n_haddr, len));
//      printf ("Address (decimal): %u\n", ifi->v4ad.n_haddr);
//      printf ("Address (hex)    : %X\n", ifi->v4ad.n_haddr);
	snprintf(temp, sizeof(temp), "%s = %d", numtoquad(ifi->v4ad.n_nmask), ifi->v4ad.n_nmaskbits);
	printf("Netmask  : \e[34m%-20s\e[0m \e[31m%s\e[0m\n", temp, numtobitmap(ifi->v4ad.n_nmask, len));
//      printf ("Netmask (hex)  - %X\n", ifi->v4ad.n_cnmask);
	printf("Wildcard : \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(ifi->v4ad.n_nmask ^ 0xffffffff),
	       numtobitmap(ifi->v4ad.n_nmask ^ 0xffffffff, len));

	printf("=>\n");

	snprintf(temp, sizeof(temp), "%s/%d", numtoquad(ifi->v4ad.n_cnaddr), ifi->v4ad.n_nmaskbits);
	if (num >= 2) {
		size_t clen = ifi->v4ad.class - 'A' + 1;
		char buf['Z' - 'A'] = { 0 };
		char *net = numtobitmap(ifi->v4ad.n_cnaddr, len);

		strncpy(buf, net, clen);
		printf("Network  : \e[34m%-20s\e[0m \e[35m%s\e[0m\e[33m%s\e[0m\n", temp, buf, &net[clen]);
		printf("HostMin  : \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(min), numtobitmap(min, len));
		printf("HostMax  : \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(max), numtobitmap(max, len));
		if (len < 31) {
			bcast = ifi->v4ad.n_broadcast;
			printf("Broadcast: \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(bcast), numtobitmap(bcast, len));
		}
	} else {
		printf("HostRoute: \e[34m%-20s\e[0m \e[33m%s\e[0m\n", numtoquad(ifi->v4ad.n_cnaddr),
		       numtobitmap(ifi->v4ad.n_cnaddr, len));
	}

	printf("Hosts/Net: \e[34m%-20u\e[0m  ", num);
	if (ifi->v4ad.class == 'I')	/* Invalid */
		printf("\e[35mClass Invalid");
	else
		printf("\e[35mClass %c", ifi->v4ad.class);
	printf("\e[0m%s\n", ifi->v4ad.class_remark);
}

char *print_comp_v6(struct sip_in6_addr addr, char *buf, size_t len)
{
	int i, j, k;
	int start, num;

	memset(buf, 0, len);
	start = -1;
	num = 0;
	j = 0;
	k = 0;
	for (i = 0; i < 8; i++) {
		if (addr.sip6_addr16[i] == 0) {
			if (j == -1)
				j = i;
			k++;
		} else {
			if (k > num && k > 1) {
				start = j;
				num = k;
			}
			j = -1;
			k = 0;
		}
	}

	if (k > num && k > 1) {
		start = j;
		num = k;
	}

	for (i = 0; i < 8; i++) {
		if (i == start) {
			if (!i)
				strlcat(buf, ":", len);
			strlcat(buf, ":", len);
			i += num - 1;
		} else {
			char tmp[5];

			snprintf(tmp, sizeof(tmp), "%x", addr.sip6_addr16[i]);
			strlcat(buf, tmp, len);

			if (i != 7)
				strlcat(buf, ":", len);
		}
	}

	return buf;
}

void print_exp_v4inv6(struct sip_in6_addr addr)
{
	unsigned char num;

	printf("\e[34m%04x:%04x:%04x:%04x:%04x:%04x:", addr.sip6_addr16[0], addr.sip6_addr16[1],
	       addr.sip6_addr16[2], addr.sip6_addr16[3], addr.sip6_addr16[4], addr.sip6_addr16[5]);

	num = (addr.sip6_addr16[6] >> 8) & 0xff;
	printf("%d.", num);
	num = addr.sip6_addr16[6] & 0xff;
	printf("%d.", num);
	num = (addr.sip6_addr16[7] >> 8) & 0xff;
	printf("%d.", num);
	num = addr.sip6_addr16[7] & 0xff;
	printf("%d\e[0m", num);
}

void print_comp_v4inv6(struct sip_in6_addr addr)
{
	unsigned char v4num;
	int i, j, k;
	int start, num;

	start = -1;
	num = 0;
	j = 0;
	k = 0;
	for (i = 0; i < 6; i++) {
		if (addr.sip6_addr16[i] == 0) {
			if (j == -1)
				j = i;
			k++;
		} else {
			if (k > num && k > 1) {
				start = j;
				num = k;
			}
			j = -1;
			k = 0;
		}
	}

	if (k > num && k > 1) {
		start = j;
		num = k;
	}

	printf("\e[34m");
	for (i = 0; i < 6; i++) {
		if (i == start) {
			if (!i)
				printf(":");
			printf(":");
			i += num - 1;
		} else {
			printf("%x:", addr.sip6_addr16[i]);
		}
	}

	v4num = (addr.sip6_addr16[6] >> 8) & 0xff;
	printf("%d.", v4num);
	v4num = addr.sip6_addr16[6] & 0xff;
	printf("%d.", v4num);
	v4num = (addr.sip6_addr16[7] >> 8) & 0xff;
	printf("%d.", v4num);
	v4num = addr.sip6_addr16[7] & 0xff;
	printf("%d", v4num);
	printf("\e[0m");
}

void print_exp_v6(struct sip_in6_addr addr)
{
	printf("\e[34m%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\e[0m",
	       addr.sip6_addr16[0], addr.sip6_addr16[1], addr.sip6_addr16[2], addr.sip6_addr16[3],
	       addr.sip6_addr16[4], addr.sip6_addr16[5], addr.sip6_addr16[6], addr.sip6_addr16[7]);
}

char *print_mixed_v6(struct sip_in6_addr addr, char *buf, size_t len)
{
	snprintf(buf, len, "%x:%x:%x:%x:%x:%x:%x:%x",
		 addr.sip6_addr16[0], addr.sip6_addr16[1], addr.sip6_addr16[2], addr.sip6_addr16[3],
		 addr.sip6_addr16[4], addr.sip6_addr16[5], addr.sip6_addr16[6], addr.sip6_addr16[7]);

	return buf;
}

static char *revdnsv6(struct sip_in6_addr addr, char *buf, size_t len)
{
	char tmp[40];
	int i, j = 0;

	memset(buf, 0, len);
	snprintf(tmp, sizeof(tmp), "%04x%04x%04x%04x%04x%04x%04x%04x",
		 addr.sip6_addr16[0], addr.sip6_addr16[1], addr.sip6_addr16[2], addr.sip6_addr16[3],
		 addr.sip6_addr16[4], addr.sip6_addr16[5], addr.sip6_addr16[6], addr.sip6_addr16[7]);

	for (i = (strlen(tmp) - 1); i >= 0; i--) {
		buf[j] = tmp[i];
		buf[j + 1] = '.';
		j += 2;
	}

	strlcat(buf, "ip6.arpa.", len);

	return buf;
}

void print_rev_v6(struct if_info *ifi)
{
	char buf[256];

	printf("Reverse DNS     : \e[34m%s\e[0m\n", revdnsv6(ifi->v6ad.haddr, buf, sizeof(buf)));
}

void print_v6(struct if_info *ifi)
{
	char buf[42];

	printf("Address ID      : \e[34m%s/%d\e[0m\n", print_mixed_v6(ifi->v6ad.suffix, buf, sizeof(buf)), ifi->v6ad.nmaskbits);
	printf("Compressed IPv6 : \e[34m%s\e[0m\n", print_comp_v6(ifi->v6ad.haddr, buf, sizeof(buf)));
	printf("Expanded IPv6   : "); print_exp_v6(ifi->v6ad.haddr);  printf("\n");
	printf("Prefix address  : \e[34m%s\e[0m = length \e[34m%d\e[0m\n", print_mixed_v6(ifi->v6ad.nmask, buf, sizeof(buf)), ifi->v6ad.nmaskbits);
	printf("Address type    : \e[35m%s\e[0m", ifi->v6ad.class_remark);
	if (ifi->v6ad.comment[0])
		printf(", %s\n", ifi->v6ad.comment);
	else
		printf("\n");
	printf("=>\n");
	printf("Network         : \e[34m%s/%d\e[0m\n", print_mixed_v6(ifi->v6ad.prefix, buf, sizeof(buf)), ifi->v6ad.nmaskbits);
	printf("HostMin         : "); print_exp_v6(ifi->v6ad.prefix);    printf("\n");
	printf("HostMax         : "); print_exp_v6(ifi->v6ad.broadcast); printf("\n");
}

void print_v4inv6(struct if_info *ifi)
{
	if (ifi->v6ad.type == V6TYPE_V4INV6 && !ifi->v6ad.real_v4) {
		printf("Address was submitted as an IPv4-compatible IPv6 address, but\n");
		printf("does not qualify as one, per the guidelines in RFC2373.\n\n");
	}

	printf("Compr. v4inv6   : "); print_comp_v4inv6(ifi->v6ad.haddr); printf("\n");
	printf("Expanded v4inv6 : "); print_exp_v4inv6(ifi->v6ad.haddr);  printf("\n");
	if (ifi->v6ad.type == V6TYPE_V4INV6 && ifi->v6ad.real_v4 == 1)
		printf("Comment			- IPv4-compatible IPv6 address\n");
	if (ifi->v6ad.type == V6TYPE_V4INV6 && ifi->v6ad.real_v4 == 2)
		printf("Comment			- IPv4-mapped IPv6 address\n");
}

int v6plus(struct sip_in6_addr *a, struct sip_in6_addr *b)
{
	int i, j, k;

	for (i = 7; i >= 0; i--) {
		if (a->sip6_addr16[i] + b->sip6_addr16[i] > 0xffff) {
			j = i - 1;
			k = 0;
			while (j >= 0 && !k) {
				k = 1;
				if (a->sip6_addr16[j] + 1 > 0xffff) {
					a->sip6_addr16[j] = 0;
					k = 0;
				} else {
					a->sip6_addr16[j]++;
				}

				j--;
			}

			a->sip6_addr16[i] = a->sip6_addr16[i] + b->sip6_addr16[i] - 0x10000;
		} else {
			a->sip6_addr16[i] += b->sip6_addr16[i];
		}
	}

	return 0;
}

void show_split_networks_v6(struct if_info *ifi, struct sip_in6_addr splitmask, int v6args, struct misc_args m_argv6)
{
	int i, j, k;
	struct sip_in6_addr sdiff, ediff, start, end, tmpaddr;

	i = 0;
	j = 0;
	do {
		if (splitmask.sip6_addr16[i] > ifi->v6ad.nmask.sip6_addr16[i])
			j = 1;
		if (ifi->v6ad.nmask.sip6_addr16[i] > splitmask.sip6_addr16[i])
			j = 2;
		i++;
	} while (i < 8 && !j);

	if (j == 2) {
		warnx("Oversized splitmask");
		return;
	}

	for (i = 0; i < 8; i++) {
		if (splitmask.sip6_addr16)
			sdiff.sip6_addr16[i] = 0xffffffff - splitmask.sip6_addr16[i];
		start.sip6_addr16[i] = ifi->v6ad.prefix.sip6_addr16[i];
		end.sip6_addr16[i] = ifi->v6ad.prefix.sip6_addr16[i] + sdiff.sip6_addr16[i];
		ediff.sip6_addr16[i] = sdiff.sip6_addr16[i];
	}

	for (i = 0; i < 8; i++)
		tmpaddr.sip6_addr16[i] = 0;
	tmpaddr.sip6_addr16[7] = 1;
	v6plus(&sdiff, &tmpaddr);

	k = 0;
	for (i = 0; i < 8; i++) {
		uint16_t val = splitmask.sip6_addr16[i];

		for (j = 0; j < 16; j++)
			k += (val & (1 << j)) ? 1 : 0;
	}

	printf("\n[Split network/%d]\n", k);

	i = 0;
	while (!i) {
		printf("Network         : "); print_exp_v6(start); printf("\n");
		printf("                  "); print_exp_v6(end);   printf("\n");

		v6plus(&start, &sdiff);

		j = 0;
		for (k = 0; k < 8; k++)
			if (end.sip6_addr16[k] != 0xffff)
				j = 1;
		if (!j)
			i = 1;

		j = 0;
		k = 0;
		do {
			if (end.sip6_addr16[k] > ifi->v6ad.broadcast.sip6_addr16[k])
				j = 1;
			if (ifi->v6ad.broadcast.sip6_addr16[k] > end.sip6_addr16[k])
				j = 2;
			k++;
		} while (k < 8 && !j);

		if (!j || j == 1)
			i = 1;

		for (k = 0; k < 8; k++)
			end.sip6_addr16[k] = 0;

		v6plus(&end, &start);
		v6plus(&end, &ediff);
	}
}

int usage(int code)
{
	printf("Usage: %s [OPTIONS] <NETWORK/LEN | - | NETWORK NETMASK>\n"
	       "\n"
	       "Global options:\n"
	       "  -h       This help text\n"
	       "  -v       Show version information\n"
	       "\n"
	       "IPv4 options:\n"
	       "  -s MASK  Split the IPv4 network into subnets of MASK size\n"
	       "\n"
	       "IPv6 options:\n"
	       "  -e       IPv4 compatible IPv6 information\n"
	       "  -r       IPv6 reverse DNS output\n"
	       "  -S MASK  Split the IPv6 network into subnets of MASK size\n"
	       "\n"
	       "Copyright (C) 2003-2013  Simon Ekstrand\n"
	       "Copyright (C) 2010-2015  Joachim Nilsson\n"
	       "\n"
	       "This is free software, under the 3-clause BSD license: you are free to change\n"
	       "and redistribute it.  There is NO WARRANTY, to the extent permitted by law\n", ident);

	return code;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
