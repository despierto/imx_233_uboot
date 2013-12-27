/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

#define NET_CONFIG_OVER_DHCP //*-AW-*/
#define SYSTEM_CTX_INFO //*-AW-*/

//extern gd_t *gd;
extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

#ifdef SYSTEM_CTX_INFO
static int system_ctx_info (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[]);
#endif

#ifdef NET_CONFIG_OVER_DHCP
static int net_config_over_dhcp (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[]);
#endif

static int netboot_common (proto_t, cmd_tbl_t *, int , char *[]);

int do_bootp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (BOOTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	bootp,	3,	1,	do_bootp,
	"boot image via network using BOOTP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (TFTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	tftpboot,	3,	1,	do_tftpb,
	"boot image via network using TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

int do_rarpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (RARP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	rarpboot,	3,	1,	do_rarpb,
	"boot image via network using RARP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

#if defined(CONFIG_CMD_DHCP)
int do_dhcp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	dhcp,	3,	1,	do_dhcp,
	"boot image via network using DHCP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#ifdef SYSTEM_CTX_INFO
int wrapper_system_ctx_info (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	
	return system_ctx_info(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
    sysinf, 3, 1, wrapper_system_ctx_info,
    "system information",
    "[none]"
);
#endif

#ifdef NET_CONFIG_OVER_DHCP
int wrapper_net_config_over_dhcp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	
	return net_config_over_dhcp(DHCP, cmdtp, argc, argv);
}


U_BOOT_CMD(
    dhclient, 3, 1, wrapper_net_config_over_dhcp,
    "configuration u-boot network parameters using DHCP protocol",
    "[none]"
);
#endif
#endif


#if defined(CONFIG_CMD_NFS)
int do_nfs (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common(NFS, cmdtp, argc, argv);
}

U_BOOT_CMD(
	nfs,	3,	1,	do_nfs,
	"boot image via network using NFS protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

static void netboot_update_env (void)
{
	char tmp[22];

	if (NetOurGatewayIP) {
		ip_to_string (NetOurGatewayIP, tmp);
		setenv ("gatewayip", tmp);
	}

	if (NetOurSubnetMask) {
		ip_to_string (NetOurSubnetMask, tmp);
		setenv ("netmask", tmp);
	}

	if (NetOurHostName[0])
		setenv ("hostname", NetOurHostName);

	if (NetOurRootPath[0])
		setenv ("rootpath", NetOurRootPath);

	if (NetOurIP) {
		ip_to_string (NetOurIP, tmp);
		setenv ("ipaddr", tmp);
	}

	if (NetServerIP) {
		ip_to_string (NetServerIP, tmp);
		setenv ("serverip", tmp);
	}

	if (NetOurDNSIP) {
		ip_to_string (NetOurDNSIP, tmp);
		setenv ("dnsip", tmp);
	}
#if defined(CONFIG_BOOTP_DNS2)
	if (NetOurDNS2IP) {
		ip_to_string (NetOurDNS2IP, tmp);
		setenv ("dnsip2", tmp);
	}
#endif
	if (NetOurNISDomain[0])
		setenv ("domain", NetOurNISDomain);

#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_TIMEOFFSET)
	if (NetTimeOffset) {
		sprintf (tmp, "%d", NetTimeOffset);
		setenv ("timeoffset", tmp);
	}
#endif
#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_NTPSERVER)
	if (NetNtpServerIP) {
		ip_to_string (NetNtpServerIP, tmp);
		setenv ("ntpserverip", tmp);
	}
#endif
}


#if defined(CONFIG_CMD_NET)
#ifdef SYSTEM_CTX_INFO
static int system_ctx_info (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[])
{
    bd_t *bd = gd->bd;

    printf("\n-------------------------------------------\n");
    printf("-----        system_ctx_info         ------\n");
    printf("-------------------------------------------\n");
    printf("\n");
    printf("Sys config:\n");
    printf("   Global desc:    (0x%X)\n", (unsigned int)gd);
    printf("   - flags:        (0x%X)\n", (unsigned int)gd->flags);
    printf("   - baudrate:     (0x%X)\n", (unsigned int)gd->baudrate);
    printf("   - have_console: (0x%X)\n", (unsigned int)gd->have_console);
    printf("   - reloc_off:    (0x%X)\n", (unsigned int)gd->reloc_off);
    printf("   - env_addr:     (0x%X)\n", (unsigned int)gd->env_addr);
    printf("   - env_valid:    (0x%X)\n", (unsigned int)gd->env_valid);
    printf("   - fb_base:      (0x%X)\n", (unsigned int)gd->fb_base);
#ifdef CONFIG_VFD
    printf("   - vfd_type:     (0x%X)\n", (unsigned int)gd->vfd_type);
#endif
    printf("\n");

    printf("   Board desc:     (0x%X)\n", (unsigned int)bd);
    if (bd) {
        printf("   - bi_ip_addr:   (0x%X): %d.%d.%d.%d\n", (unsigned int)bd->bi_ip_addr,
                        (unsigned int)(bd->bi_ip_addr&0xFF), (unsigned int)((bd->bi_ip_addr>>8)&0xFF), 
                        (unsigned int)((bd->bi_ip_addr>>16)&0xFF), (unsigned int)((bd->bi_ip_addr>>24)&0xFF));
        printf("   - bi_env:       (0x%X)\n", (unsigned int)bd->bi_env);
        printf("   - bi_dram[]:    (0x%X), size=%d\n", (unsigned int)bd->bi_dram, CONFIG_NR_DRAM_BANKS);
        printf("      - start:     (0x%X)\n", (unsigned int)bd->bi_dram[0].start);
        printf("      - size:      (0x%X)\n", (unsigned int)bd->bi_dram[0].size);
    }
    printf("\n");
    printf("-------------------------------------------\n");
    printf("   Net config:\n");
    printf("   - NetOurIP:          (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetOurIP, 
            (unsigned int)(NetOurIP&0xFF), (unsigned int)((NetOurIP>>8)&0xFF), (unsigned int)((NetOurIP>>16)&0xFF), (unsigned int)((NetOurIP>>24)&0xFF));
    printf("   - NetOurGatewayIP:   (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetOurGatewayIP, 
            (unsigned int)(NetOurGatewayIP&0xFF), (unsigned int)((NetOurGatewayIP>>8)&0xFF), (unsigned int)((NetOurGatewayIP>>16)&0xFF), (unsigned int)((NetOurGatewayIP>>24)&0xFF));
    printf("   - NetOurSubnetMask:  (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetOurSubnetMask, 
            (unsigned int)(NetOurSubnetMask&0xFF), (unsigned int)((NetOurSubnetMask>>8)&0xFF), (unsigned int)((NetOurSubnetMask>>16)&0xFF), (unsigned int)((NetOurSubnetMask>>24)&0xFF));
    printf("   - NetServerIP:       (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetServerIP, 
            (unsigned int)(NetServerIP&0xFF), (unsigned int)((NetServerIP>>8)&0xFF), (unsigned int)((NetServerIP>>16)&0xFF), (unsigned int)((NetServerIP>>24)&0xFF));
    printf("   - NetOurNativeVLAN:  (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetOurNativeVLAN, 
            (unsigned int)(NetOurNativeVLAN&0xFF), (unsigned int)((NetOurNativeVLAN>>8)&0xFF), (unsigned int)((NetOurNativeVLAN>>16)&0xFF), (unsigned int)((NetOurNativeVLAN>>24)&0xFF));
    printf("   - NetOurVLAN:        (0x%8X): %d.%d.%d.%d\n", (unsigned int)NetOurVLAN, 
            (unsigned int)(NetOurVLAN&0xFF), (unsigned int)((NetOurVLAN>>8)&0xFF), (unsigned int)((NetOurVLAN>>16)&0xFF), (unsigned int)((NetOurVLAN>>24)&0xFF));
    printf("   - NetOurDNSIP:       (0x8%X): %d.%d.%d.%d\n", (unsigned int)NetOurDNSIP, 
            (unsigned int)(NetOurDNSIP&0xFF), (unsigned int)((NetOurDNSIP>>8)&0xFF), (unsigned int)((NetOurDNSIP>>16)&0xFF), (unsigned int)((NetOurDNSIP>>24)&0xFF));
    printf("\n");
    printf("-------------------------------------------\n");
    printf("   U boot settings:\n");

    printf("\n");
    printf("-------------------------------------------\n");
}
#endif


#ifdef NET_CONFIG_OVER_DHCP
static int
    net_config_over_dhcp (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[])
    {
    char *s;
    char *end;
    int   rcode = 0;
    int   size;
    ulong addr;
    bd_t *bd = gd->bd;

    printf("\n-------------------------------------------\n");
    printf("-----      net_config_over_dhcp      ------\n");
    printf("-------------------------------------------\n");

    /* pre-set load_addr */
    if ((s = getenv("loadaddr")) != NULL) {
    	load_addr = simple_strtoul(s, NULL, 16);
    }

    switch (argc) {
    case 1:
    	break;

    case 2:	/*
    	 * Only one arg - accept two forms:
    	 * Just load address, or just boot file name. The latter
    	 * form must be written in a format which can not be
    	 * mis-interpreted as a valid number.
    	 */
    	addr = simple_strtoul(argv[1], &end, 16);
    	if (end == (argv[1] + strlen(argv[1])))
    		load_addr = addr;
    	else
    		copy_filename(BootFile, argv[1], sizeof(BootFile));
    	break;

    case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
    	copy_filename (BootFile, argv[2], sizeof(BootFile));

    	break;

    default: cmd_usage(cmdtp);
    	show_boot_progress (-80);
    	return 1;
    }

    show_boot_progress (80);
    printf("[net_config_over_dhcp] net_config_over_dhcp\n");
    if ((size = NetLoop(proto)) < 0) {

        printf("[net_config_over_dhcp] show_boot_progress 1\n");        
        show_boot_progress (-81);
        return 1;
    }

    printf("[net_config_over_dhcp] show_boot_progress 2\n");        
    show_boot_progress (81);
    /* NetLoop ok, update environment */

    printf("[net_config_over_dhcp] netboot_update_env\n");        
    netboot_update_env();

    /* done if no file was loaded (no errors though) */
    if (size == 0) {
    	show_boot_progress (-82);
    	return 0;
    }

    /* flush cache */
     printf("[net_config_over_dhcp] flush_cache\n");        
    flush_cache(load_addr, size);

    /* Loading ok, check if we should attempt an auto-start */
    if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
    	char *local_args[2];
    	local_args[0] = argv[0];
    	local_args[1] = NULL;

    	printf ("Automatic boot of image at addr 0x%08lX ...\n",
    		load_addr);
    	show_boot_progress (82);
    	rcode = do_bootm (cmdtp, 0, 1, local_args);
    }

#ifdef CONFIG_SOURCE
    if (((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)) {
    	printf ("Running \"source\" command at addr 0x%08lX",
    		load_addr);

    	s = getenv ("autoscript_uname");
    	if (s)
    		printf (":%s ...\n", s);
    	else
    		puts (" ...\n");

    	show_boot_progress (83);
    	rcode = source (load_addr, s);
    }
#endif
    if (rcode < 0)
    	show_boot_progress (-83);
    else
    	show_boot_progress (84);
    return rcode;
    }
#endif
#endif


#if defined(CONFIG_CMD_PING)
int do_ping (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		return -1;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0) {
		cmd_usage(cmdtp);
		return -1;
	}

	if (NetLoop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	printf("host %s is alive\n", argv[1]);

	return 0;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"send ICMP ECHO_REQUEST to network host",
	"pingAddress"
);
#endif

#if defined(CONFIG_CMD_CDP)

static void cdp_update_env(void)
{
	char tmp[16];

	if (CDPApplianceVLAN != htons(-1)) {
		printf("CDP offered appliance VLAN %d\n", ntohs(CDPApplianceVLAN));
		VLAN_to_string(CDPApplianceVLAN, tmp);
		setenv("vlan", tmp);
		NetOurVLAN = CDPApplianceVLAN;
	}

	if (CDPNativeVLAN != htons(-1)) {
		printf("CDP offered native VLAN %d\n", ntohs(CDPNativeVLAN));
		VLAN_to_string(CDPNativeVLAN, tmp);
		setenv("nvlan", tmp);
		NetOurNativeVLAN = CDPNativeVLAN;
	}

}

int do_cdp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int r;

	r = NetLoop(CDP);
	if (r < 0) {
		printf("cdp failed; perhaps not a CISCO switch?\n");
		return 1;
	}

	cdp_update_env();

	return 0;
}

U_BOOT_CMD(
	cdp,	1,	1,	do_cdp,
	"Perform CDP network configuration",
);
#endif


static int
netboot_common (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	char *s;
	char *end;
	int   rcode = 0;
	int   size;
	ulong addr;

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
	case 1:
		break;

	case 2:	/*
		 * Only one arg - accept two forms:
		 * Just load address, or just boot file name. The latter
		 * form must be written in a format which can not be
		 * mis-interpreted as a valid number.
		 */
		addr = simple_strtoul(argv[1], &end, 16);
		if (end == (argv[1] + strlen(argv[1])))
			load_addr = addr;
		else
			copy_filename(BootFile, argv[1], sizeof(BootFile));
		break;

	case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
		copy_filename (BootFile, argv[2], sizeof(BootFile));

		break;

	default: cmd_usage(cmdtp);
		show_boot_progress (-80);
		return 1;
	}

	show_boot_progress (80);
	if ((size = NetLoop(proto)) < 0) {
		show_boot_progress (-81);
		return 1;
	}

	show_boot_progress (81);
	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0) {
		show_boot_progress (-82);
		return 0;
	}

	/* flush cache */
	flush_cache(load_addr, size);

	/* Loading ok, check if we should attempt an auto-start */
	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);
		show_boot_progress (82);
		rcode = do_bootm (cmdtp, 0, 1, local_args);
	}

#ifdef CONFIG_SOURCE
	if (((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)) {
		printf ("Running \"source\" command at addr 0x%08lX",
			load_addr);

		s = getenv ("autoscript_uname");
		if (s)
			printf (":%s ...\n", s);
		else
			puts (" ...\n");

		show_boot_progress (83);
		rcode = source (load_addr, s);
	}
#endif
	if (rcode < 0)
		show_boot_progress (-83);
	else
		show_boot_progress (84);
	return rcode;
}


#if defined(CONFIG_CMD_SNTP)
int do_sntp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *toff;

	if (argc < 2) {
		NetNtpServerIP = getenv_IPaddr ("ntpserverip");
		if (NetNtpServerIP == 0) {
			printf ("ntpserverip not set\n");
			return (1);
		}
	} else {
		NetNtpServerIP = string_to_ip(argv[1]);
		if (NetNtpServerIP == 0) {
			printf ("Bad NTP server IP address\n");
			return (1);
		}
	}

	toff = getenv ("timeoffset");
	if (toff == NULL) NetTimeOffset = 0;
	else NetTimeOffset = simple_strtol (toff, NULL, 10);

	if (NetLoop(SNTP) < 0) {
		printf("SNTP failed: host %s not responding\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	sntp,	2,	1,	do_sntp,
	"synchronize RTC via network",
	"[NTP server IP]\n"
);
#endif

#if defined(CONFIG_CMD_DNS)
int do_dns(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc == 1) {
		cmd_usage(cmdtp);
		return -1;
	}

	/*
	 * We should check for a valid hostname:
	 * - Each label must be between 1 and 63 characters long
	 * - the entire hostname has a maximum of 255 characters
	 * - only the ASCII letters 'a' through 'z' (case-insensitive),
	 *   the digits '0' through '9', and the hyphen
	 * - cannot begin or end with a hyphen
	 * - no other symbols, punctuation characters, or blank spaces are
	 *   permitted
	 * but hey - this is a minimalist implmentation, so only check length
	 * and let the name server deal with things.
	 */
	if (strlen(argv[1]) >= 255) {
		printf("dns error: hostname too long\n");
		return 1;
	}

	NetDNSResolve = argv[1];

	if (argc == 3)
		NetDNSenvvar = argv[2];
	else
		NetDNSenvvar = NULL;

	if (NetLoop(DNS) < 0) {
		printf("dns lookup of %s failed, check setup\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	dns,	3,	1,	do_dns,
	"lookup the IP of a hostname",
	"hostname [envvar]"
);

#endif	/* CONFIG_CMD_DNS */
