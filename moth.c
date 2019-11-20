/* This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Fredrik Hallenberg <megahallon@gmail.com> */
/* Based on phytool: https://github.com/wkz/phytool */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>

#include <linux/mdio.h>
#include <linux/sockios.h>

#define REGCR 0xd
#define ADDAR 0xe

#define MMDREG(mmd, reg) ((mmd << 16) | (reg))
#define LD_CTRL MMDREG(0x1f, 0x400)
#define LDG_CTRL1 MMDREG(0x1f, 0x401)
#define LED_CFG2 MMDREG(0x1f, 0x469)
#define PMA_CTRL2 MMDREG(0x01, 0x834)
#define MASTER_BIT (1 << 14)


static int __phy_op(const char *ifc, uint16_t reg, uint16_t *val, int cmd)
{
	static int sock = -1;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_ioctl_data *)(&ifr.ifr_data);
	int err;

	if (sock < 0)
		sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return sock;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifc, sizeof(ifr.ifr_name));
	mii->reg_num = reg;
	mii->val_in = *val;

	err = ioctl(sock, cmd, &ifr);
	if (err)
		return -errno;

	*val = mii->val_out;
	return 0;
}

static int __phy_read(const char *ifc, uint16_t reg)
{
	uint16_t val = 0;
	int err = __phy_op(ifc, reg, &val, SIOCGMIIREG);

	if (err) {
		fprintf(stderr, "error: phy_read (%d)\n", err);
		return err;
	}

	return val;
}

static int __phy_write(const char *ifc, uint16_t reg, uint16_t val)
{
	int err = __phy_op(ifc, reg, &val, SIOCSMIIREG);

	if (err)
		fprintf(stderr, "error: phy_write (%d)\n", err);

	return err;
}

static int phy_read(const char *ifc, uint32_t mmd_reg)
{
	uint16_t mmd = mmd_reg >> 16;
	uint16_t reg = mmd_reg & 0xffff;
	if (reg > 0x1f || mmd != 0x1f) {
		__phy_write(ifc, REGCR, mmd);
		__phy_write(ifc, ADDAR, reg);
		__phy_write(ifc, REGCR, 0x4000 | mmd);
		return __phy_read(ifc, ADDAR);
	}
	return __phy_read(ifc, reg);
}

static int phy_write(const char *ifc, uint32_t mmd_reg, uint16_t val)
{
	uint16_t mmd = mmd_reg >> 16;
	uint16_t reg = mmd_reg & 0xffff;
	if (reg > 0x1f || mmd != 0x1f) {
		__phy_write(ifc, REGCR, mmd);
		__phy_write(ifc, ADDAR, reg);
		__phy_write(ifc, REGCR, 0x4000 | mmd);
		return __phy_write(ifc, ADDAR, val);
	}
	return __phy_write(ifc, reg, val);
}

int main(int argc, char *argv[])
{
	if (argc < 3 || argc > 5) {
		fprintf(stderr, "usage: %s <interface> <command>\n", argv[0]);
		fprintf(stderr,
			"dump\n"
			"read_ms\n"
			"set_master\n"
			"set_slave\n"
			"read_line_driver\n"
			"set_termination <value> 0=70ohm 0x1f=34.4ohm\n"
			"set_swing <value> 0=-16%% gain, 8=0%% gain, 15=14%% gain\n"
			"read <mmd 1f reg>\n"
			"read_mmd1 <mmd 1 reg>\n"
			"write <mmd 1f reg> <value>\n"
			"write_mmd1 <mmd 1 reg> <value>\n");
		return 0;
	}

	char *ifc = argv[1];
	char *cmd = argv[2];
	char *arg = argv[3];
	char *arg2 = argv[4];
	int ctrl2 = phy_read(ifc, PMA_CTRL2);
	if (ctrl2 < 0)
		return 1;

	if (strcmp(cmd, "read_ms") == 0) {
		printf("T1 mode: %s\n", (ctrl2 & MASTER_BIT) ? "master" : "slave");
	}
	else if (strcmp(cmd, "set_master") == 0) {
		phy_write(ifc, PMA_CTRL2, ctrl2 | MASTER_BIT);
	}
	else if (strcmp(cmd, "set_slave") == 0) {
		phy_write(ifc, PMA_CTRL2, ctrl2 & ~MASTER_BIT);
	}
	else if (strcmp(cmd, "read_line_driver") == 0) {
		int ld_ctrl = phy_read(ifc, LD_CTRL);
		int ldg_ctrl = phy_read(ifc, LDG_CTRL1);
		printf("Series termination: 0x%x\n", ld_ctrl >> 8);
		printf("Swing control: 0x%x\n", ldg_ctrl);
	}
	else if (strcmp(cmd, "set_termination") == 0) {
		uint16_t value = strtoul(arg, NULL, 0);
		phy_write(ifc, LD_CTRL, value << 8);
	}
	else if (strcmp(cmd, "set_swing") == 0) {
		uint16_t value = strtoul(arg, NULL, 0);
		phy_write(ifc, LDG_CTRL1, value);
	}
	else if (strcmp(cmd, "read") == 0) {
		uint16_t reg = strtoul(arg, NULL, 0);
		int out = phy_read(ifc, MMDREG(0x1f, reg));
		printf("0x%x => 0x%x\n", reg, out);
	}
	else if (strcmp(cmd, "read_mmd1") == 0) {
		uint16_t reg = strtoul(arg, NULL, 0);
		int out = phy_read(ifc, MMDREG(0x1, reg));
		printf("0x%x => 0x%x\n", reg, out);
	}
	else if (strcmp(cmd, "write") == 0) {
		uint16_t reg = strtoul(arg, NULL, 0);
		uint16_t value = strtoul(arg2, NULL, 0);
		phy_write(ifc, MMDREG(0x1f, reg), value);
		printf("0x%x = 0x%x\n", reg, value);
	}
	else if (strcmp(cmd, "write_mmd1") == 0) {
		uint16_t reg = strtoul(arg, NULL, 0);
		uint16_t value = strtoul(arg2, NULL, 0);
		phy_write(ifc, MMDREG(0x1, reg), value);
		printf("0x%x = 0x%x\n", reg, value);
	}
	else if (strcmp(cmd, "dump") == 0) {
		struct {
			char desc[64];
			uint32_t reg;
		} dump[] = {
			{"bmcr", MMDREG(0x1f, 0)},
			{"bmsr", MMDREG(0x1f, 1)},
			{"int stat 1", MMDREG(0x1f, 0x12)},
			{"int stat 2", MMDREG(0x1f, 0x13)},
			{"false carrier sense counter", MMDREG(0x1f, 0x14)},
			{"receive error counter", MMDREG(0x1f, 0x15)},
			{"int stat 3", MMDREG(0x1f, 0x18)},
			{"link status", MMDREG(0x1f, 0x133)},
			{"snr", MMDREG(0x1f, 0x197)},
			{"sqi", MMDREG(0x1f, 0x198)},
			{"line driver termination", MMDREG(0x1f, 0x400)},
			{"line driver swing", MMDREG(0x1f, 0x401)},
			{"esd", MMDREG(0x1f, 0x448)},
			{"", 0}
		};

		int i = 0;
		while (1) {
			if (dump[i].reg == 0)
				break;
			int v = phy_read(ifc, dump[i].reg);
			printf("%s (0x%x): 0x%x\n", dump[i].desc, dump[i].reg & 0xffff, v);
			++i;
		}
	}
	return 0;
}
