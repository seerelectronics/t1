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
	if (argc != 3) {
		fprintf(stderr, "usage: %s <command> <interface>\n", argv[0]);
		exit(1);
	}

	int ctrl2 = phy_read(argv[2], PMA_CTRL2);
	if (ctrl2 < 0)
	  return 1;

	if (strcmp(argv[1], "read") == 0) {
		printf("T1 mode: %s\n", (ctrl2 & MASTER_BIT) ? "master" : "slave");
	}
	else if (strcmp(argv[1], "master") == 0) {
		phy_write(argv[2], PMA_CTRL2, ctrl2 | MASTER_BIT);
	}
	else if (strcmp(argv[1], "slave") == 0) {
		phy_write(argv[2], PMA_CTRL2, ctrl2 & ~MASTER_BIT);
	}

	return 0;
}
