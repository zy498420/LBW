/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RealFD.h"
#include "termios.h"

#define LINUX_F_RDLCK         0
#define LINUX_F_WRLCK         1
#define LINUX_F_UNLCK         2

#pragma pack(push, 1)
struct linux_flock
{
	u16 l_type;
	u16 l_whence;
	u32 l_start;
	u32 l_len;
	u32 l_pid;
};

struct linux_flock64
{
	u16 l_type;
	u16 l_whence;
	u64 l_start;
	u64 l_len;
	u32 l_pid;
};
#pragma pack(pop)

static void convert(struct linux_flock& lfl, struct flock& ifl)
{
	switch (lfl.l_type)
	{
		case LINUX_F_RDLCK: ifl.l_type = F_RDLCK; break;
		case LINUX_F_WRLCK: ifl.l_type = F_WRLCK; break;
		case LINUX_F_UNLCK: ifl.l_type = F_UNLCK; break;
		default: throw EINVAL;
	}

	ifl.l_whence = lfl.l_whence;
	ifl.l_start = lfl.l_start;
	ifl.l_len = lfl.l_len;
	ifl.l_pid = lfl.l_pid;
}

static void convert(struct linux_flock64& lfl, struct flock& ifl)
{
	switch (lfl.l_type)
	{
		case LINUX_F_RDLCK: ifl.l_type = F_RDLCK; break;
		case LINUX_F_WRLCK: ifl.l_type = F_WRLCK; break;
		case LINUX_F_UNLCK: ifl.l_type = F_UNLCK; break;
		default: throw EINVAL;
	}

	if ((lfl.l_start > 0xffffffffLL) ||
		(lfl.l_len > 0xffffffffLL))
		throw ENOLCK;

	ifl.l_whence = lfl.l_whence;
	ifl.l_start = lfl.l_start;
	ifl.l_len = lfl.l_len;
	ifl.l_pid = lfl.l_pid;
}

int RealFD::Fcntl(int cmd, u_int32_t argument)
{
	int fd = GetFD();

	switch (cmd)
	{
		case LINUX_F_GETFL:
		{
			int result = fcntl(fd, F_GETFL, NULL);
			if (result == -1)
				throw errno;
			return FileFlagsI2L(result);
		}

		case LINUX_F_SETFL:
		{
			int iflags = FileFlagsL2I(argument);
			//log("setting flags for realfd %d to %x", fd, iflags);
			int result = fcntl(fd, F_SETFL, iflags);
			CheckError(result);
			return result;
		}

		case LINUX_F_SETLK:
		{
			struct linux_flock& lfl = *(struct linux_flock*) argument;
			struct flock ifl;
			convert(lfl, ifl);

			int result = fcntl(fd, F_SETLK, &ifl);
			CheckError(result);
			return result;
		}

		case LINUX_F_SETLKW:
		{
			struct linux_flock& lfl = *(struct linux_flock*) argument;
			struct flock ifl;
			convert(lfl, ifl);

			int result = fcntl(fd, F_SETLKW, &ifl);
			CheckError(result);
			return result;
		}

		case LINUX_F_SETLK64:
		{
			struct linux_flock64& lfl = *(struct linux_flock64*) argument;
			struct flock ifl;
			convert(lfl, ifl);

			int result = fcntl(fd, F_SETLK, &ifl);
			CheckError(result);
			return result;
		}

		case LINUX_F_SETLKW64:
		{
			struct linux_flock64& lfl = *(struct linux_flock64*) argument;
			struct flock ifl;
			convert(lfl, ifl);

			int result = fcntl(fd, F_SETLKW, &ifl);
			CheckError(result);
			return result;
		}
	}

	return FD::Fcntl(cmd, argument);
}
