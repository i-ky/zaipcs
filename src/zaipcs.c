#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "sysinc.h"
#include "module.h"

/* POSIX.1 requires that the caller define this union. */
/* On versions of  glibc  where  this  union  is  not  */
/* defined, the macro _SEM_SEMUN_UNDEFINED is defined. */
#ifdef _SEM_SEMUN_UNDEFINED
union semun
{
	int		val;	/* Value for SETVAL */
	struct semid_ds	*buf;	/* Buffer for IPC_STAT, IPC_SET */
	unsigned short	*array;	/* Array for GETALL, SETALL */
#ifdef IPC_INFO
	struct seminfo	*__buf;	/* Buffer for IPC_INFO (Linux-specific) */
#endif
};
#endif

/* some bureaucracy */

int	zbx_module_api_version(void)
{
#ifdef ZBX_MODULE_API_VERSION
	return ZBX_MODULE_API_VERSION;		/* state-of-the-art constant */
#else
	return ZBX_MODULE_API_VERSION_ONE;	/* outdated pre-3.2 constant */
#endif
}

int	zbx_module_init(void)
{
	return ZBX_MODULE_OK;
}

/* internal codes */

#define IPCS_OK		0
#define IPCS_FAIL	1

/* error messages */

#define IPCS_EINVALMOD	strdup("invalid 'mode' parameter")
#define IPCS_EINVALOPT	strdup("invalid 'option' parameter")
#define IPCS_EINVALRID	strdup("invalid resource identifier")
#define IPCS_ENUMPARAM	strdup("incorrect number of parameters")
#define IPCS_EPLATFORM	strdup("not supported on this platform")
#define IPCS_ESYSERROR	strdup(strerror(errno))

/* helper functions */

void	ipcs_strappf(char **old, size_t *old_size, size_t *old_offset, const char *format, ...)
{
	char	*new = *old;
	size_t	new_size = *old_size, new_offset = *old_offset;
	va_list	args;

	if (NULL == new && NULL == (new = malloc(new_size = 128)))
		return;

	va_start(args, format);
	new_offset += vsnprintf(new + new_offset, new_size - new_offset, format, args);
	va_end(args);

	if (new_offset >= new_size)
	{
		char	*renew;

		new_size = new_offset + 1;

		if (NULL != (renew = realloc(new, new_size)))
		{
			new = renew;
			va_start(args, format);
			vsnprintf(new + *old_offset, new_size - *old_offset, format, args);
			va_end(args);
		}
		else
		{
			free(new);
			new = NULL;
			new_size = new_offset = 0;
		}
	}

	*old = new;
	*old_size = new_size;
	*old_offset = new_offset;
}

/* discoveries */

#define LLD_JSON_PRE	"{"							\
				"\"data\":["
#define LLD_JSON_ROW			"{"					\
						"\"{#KEY}\":"	"\"0x%08x\","	\
						"\"{#ID}\":"	"\"%d\","	\
						"\"{#OWNER}\":"	"\"%d\","	\
						"\"{#PERMS}\":"	"\"%03o\""	\
					"}"
#define LLD_JSON_END		"]"						\
			"}"

/* NOLINTNEXTLINE(misc-unused-parameters) */
static int	ipcs_shmem_discovery(AGENT_REQUEST *request, AGENT_RESULT *result)
{
#if defined(SHM_INFO) && defined(SHM_STAT)
	const char	*delim = "";
	char		*json = NULL;
	size_t		json_size = 0, json_offset = 0;
	int		maxidx, idx, shmid;
	struct shmid_ds	shmid_ds;

	if (-1 == (maxidx = shmctl(0, SHM_INFO, &shmid_ds)))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_PRE);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	for (idx = 0; idx <= maxidx; idx++)
	{
		if (-1 == (shmid = shmctl(idx, SHM_STAT, &shmid_ds)))
			continue;

		ipcs_strappf(&json, &json_size, &json_offset, "%s" LLD_JSON_ROW, delim,
				shmid_ds.shm_perm.__key, shmid, shmid_ds.shm_perm.uid, shmid_ds.shm_perm.mode & 0777);

		if (NULL == json)
		{
			SET_MSG_RESULT(result, IPCS_ESYSERROR);
			return SYSINFO_RET_FAIL;
		}

		delim = ",";
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_END);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	SET_TEXT_RESULT(result, json);
	return SYSINFO_RET_OK;
#else
	SET_MSG_RESULT(result, IPCS_EPLATFORM);
	return SYSINFO_RET_FAIL;
#endif
}

/* NOLINTNEXTLINE(misc-unused-parameters) */
static int	ipcs_queue_discovery(AGENT_REQUEST *request, AGENT_RESULT *result)
{
#if defined(MSG_INFO) && defined(MSG_STAT)
	const char	*delim = "";
	char		*json = NULL;
	size_t		json_size = 0, json_offset = 0;
	int		maxidx, idx, msqid;
	struct msqid_ds	msqid_ds;

	if (-1 == (maxidx = msgctl(0, MSG_INFO, &msqid_ds)))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_PRE);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	for (idx = 0; idx <= maxidx; idx++)
	{
		if (-1 == (msqid = msgctl(idx, MSG_STAT, &msqid_ds)))
			continue;

		ipcs_strappf(&json, &json_size, &json_offset, "%s" LLD_JSON_ROW, delim,
				msqid_ds.msg_perm.__key, msqid, msqid_ds.msg_perm.uid, msqid_ds.msg_perm.mode & 0777);

		if (NULL == json)
		{
			SET_MSG_RESULT(result, IPCS_ESYSERROR);
			return SYSINFO_RET_FAIL;
		}

		delim = ",";
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_END);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	SET_TEXT_RESULT(result, json);
	return SYSINFO_RET_OK;
#else
	SET_MSG_RESULT(result, IPCS_EPLATFORM);
	return SYSINFO_RET_FAIL;
#endif
}

/* NOLINTNEXTLINE(misc-unused-parameters) */
static int	ipcs_semaphore_discovery(AGENT_REQUEST *request, AGENT_RESULT *result)
{
#if defined(SEM_INFO) && defined(SEM_STAT)
	const char	*delim = "";
	char		*json = NULL;
	size_t		json_size = 0, json_offset = 0;
	int		maxidx, idx, semid;
	struct semid_ds	semid_ds;
	union semun	semid_un;

	semid_un.buf = &semid_ds;

	if (-1 == (maxidx = semctl(0, 0, SEM_INFO, semid_un)))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_PRE);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	for (idx = 0; idx <= maxidx; idx++)
	{
		if (-1 == (semid = semctl(idx, 0, SEM_STAT, semid_un)))
			continue;

		ipcs_strappf(&json, &json_size, &json_offset, "%s" LLD_JSON_ROW, delim,
				semid_ds.sem_perm.__key, semid, semid_ds.sem_perm.uid, semid_ds.sem_perm.mode & 0777);

		if (NULL == json)
		{
			SET_MSG_RESULT(result, IPCS_ESYSERROR);
			return SYSINFO_RET_FAIL;
		}

		delim = ",";
	}

	ipcs_strappf(&json, &json_size, &json_offset, LLD_JSON_END);

	if (NULL == json)
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	SET_TEXT_RESULT(result, json);
	return SYSINFO_RET_OK;
#else
	SET_MSG_RESULT(result, IPCS_EPLATFORM);
	return SYSINFO_RET_FAIL;
#endif
}

/* details */

static int	ipcs_request_parse(AGENT_REQUEST *request, AGENT_RESULT *result,
		int *resourceid, const char **mode, const char **option)
{
	int	param_num;

	param_num = get_rparams_num(request);

	if (1 > param_num || 3 < param_num)
	{
		SET_MSG_RESULT(result, IPCS_ENUMPARAM);
		return IPCS_FAIL;
	}

	/* NOLINTNEXTLINE(cert-err34-c) */
	if (1 != sscanf(get_rparam(request, 0), "%d", resourceid) || 0 > *resourceid)
	{
		SET_MSG_RESULT(result, IPCS_EINVALRID);
		return IPCS_FAIL;
	}

	*mode = get_rparam(request, 1);
	*option = get_rparam(request, 2);
	return IPCS_OK;
}

static int	ipcs_shmem_details(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	const char	*mode, *option;
	int		shmid;
	struct shmid_ds	shmid_ds;

	if (IPCS_OK != ipcs_request_parse(request, result, &shmid, &mode, &option))
		return SYSINFO_RET_FAIL;

	if (-1 == shmctl(shmid, IPC_STAT, &shmid_ds))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	if (NULL == mode)
	{
		SET_MSG_RESULT(result, IPCS_EINVALMOD);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "owner"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_perm.uid);	/* Effective UID of owner */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_perm.gid);	/* Effective GID of owner */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "creator"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_perm.cuid);	/* Effective UID of creator */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_perm.cgid);	/* Effective GID of creator */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "status"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "dest"))
		{
#ifdef SHM_DEST
			SET_UI64_RESULT(result, shmid_ds.shm_perm.mode & SHM_DEST);	/* SHM_DEST flag */
			return SYSINFO_RET_OK;
#else
			SET_MSG_RESULT(result, IPCS_EPLATFORM);
			return SYSINFO_RET_FAIL;
#endif
		}

		if (0 == strcmp(option, "locked"))
		{
#ifdef SHM_LOCKED
			SET_UI64_RESULT(result, shmid_ds.shm_perm.mode & SHM_LOCKED);	/* SHM_LOCKED flag */
			return SYSINFO_RET_OK;
#else
			SET_MSG_RESULT(result, IPCS_EPLATFORM);
			return SYSINFO_RET_FAIL;
#endif
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "permissions"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, shmid_ds.shm_perm.mode & 0777);	/* Permissions */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "size"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, shmid_ds.shm_segsz);	/* Size of segment (bytes) */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "time"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "attach"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_atime);	/* Last attach time */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "detach"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_dtime);	/* Last detach time */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "change"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_ctime);	/* Last change time */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "pid"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "creator"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_cpid);	/* PID of creator */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "last"))
		{
			SET_UI64_RESULT(result, shmid_ds.shm_lpid);	/* PID of last shmat(2)/shmdt(2) */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "nattch"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, shmid_ds.shm_nattch);	/* No. of current attaches */
		return SYSINFO_RET_OK;
	}

	SET_MSG_RESULT(result, IPCS_EINVALMOD);
	return SYSINFO_RET_FAIL;
}

static int	ipcs_queue_details(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	const char	*mode, *option;
	int		msqid;
	struct msqid_ds	msqid_ds;

	if (IPCS_OK != ipcs_request_parse(request, result, &msqid, &mode, &option))
		return SYSINFO_RET_FAIL;

	if (-1 == msgctl(msqid, IPC_STAT, &msqid_ds))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	if (NULL == mode)
	{
		SET_MSG_RESULT(result, IPCS_EINVALMOD);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "owner"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_perm.uid);	/* Effective UID of owner */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_perm.gid);	/* Effective GID of owner */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "creator"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_perm.cuid);	/* Effective UID of creator */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_perm.cgid);	/* Effective GID of creator */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "permissions"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, msqid_ds.msg_perm.mode & 0777);	/* Permissions */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "time"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "send"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_stime);	/* Time of last msgsnd(2) */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "receive"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_rtime);	/* Time of last msgrcv(2) */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "change"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_ctime);	/* Time of last change */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "messages"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, msqid_ds.msg_qnum);	/* Current number of messages in queue */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "size"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, msqid_ds.msg_qbytes);	/* Maximum number of bytes allowed in queue */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "pid"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "send"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_lspid);	/* PID of last msgsnd(2) */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "receive"))
		{
			SET_UI64_RESULT(result, msqid_ds.msg_lrpid);	/* PID of last msgrcv(2) */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	SET_MSG_RESULT(result, IPCS_EINVALMOD);
	return SYSINFO_RET_FAIL;
}

static int	ipcs_semaphore_loop(int semid, int nsems, int cmd, const char *option, AGENT_RESULT *result)
{
	union semun	semid_un;

	if (NULL == option || '\0' == *option || 0 == strcmp(option, "sum"))
	{
		int	i, res, sum = 0;

		for (i = 0; i < nsems; i++)
		{
			if (-1 == (res = semctl(semid, i, cmd, semid_un)))
				goto syserr;

			sum += res;
		}

		SET_UI64_RESULT(result, sum);
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(option, "max"))
	{
		int	i, res, max = 0;

		for (i = 0; i < nsems; i++)
		{
			if (-1 == (res = semctl(semid, i, cmd, semid_un)))
				goto syserr;

			if (res > max)
				max = res;
		}

		SET_UI64_RESULT(result, max);
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(option, "idx"))
	{
		int	i, res, idx = 0, max = 0;

		for (i = 0; i < nsems; i++)
		{
			if (-1 == (res = semctl(semid, i, cmd, semid_un)))
				goto syserr;

			if (res > max)
				max = res, idx = i;
		}

		SET_UI64_RESULT(result, idx);
		return SYSINFO_RET_OK;
	}

	SET_MSG_RESULT(result, IPCS_EINVALOPT);
	return SYSINFO_RET_FAIL;
syserr:
	SET_MSG_RESULT(result, IPCS_ESYSERROR);
	return SYSINFO_RET_FAIL;
}

static int	ipcs_semaphore_details(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	const char	*mode, *option;
	int		semid;
	struct semid_ds	semid_ds;
	union semun	semid_un;

	if (IPCS_OK != ipcs_request_parse(request, result, &semid, &mode, &option))
		return SYSINFO_RET_FAIL;

	semid_un.buf = &semid_ds;

	if (-1 == semctl(semid, 0, IPC_STAT, semid_un))
	{
		SET_MSG_RESULT(result, IPCS_ESYSERROR);
		return SYSINFO_RET_FAIL;
	}

	if (NULL == mode)
	{
		SET_MSG_RESULT(result, IPCS_EINVALMOD);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "owner"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_perm.uid);	/* Effective UID of owner */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_perm.gid);	/* Effective GID of owner */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "creator"))
	{
		if (NULL == option || '\0' == *option || 0 == strcmp(option, "user"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "group"))
		{
			//TODO
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "uid"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_perm.cuid);	/* Effective UID of creator */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "gid"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_perm.cgid);	/* Effective GID of creator */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "permissions"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, semid_ds.sem_perm.mode & 0777);	/* Permissions */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "time"))
	{
		if (NULL == option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		if (0 == strcmp(option, "semop"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_otime);	/* Last semop time */
			return SYSINFO_RET_OK;
		}

		if (0 == strcmp(option, "change"))
		{
			SET_UI64_RESULT(result, semid_ds.sem_ctime);	/* Last change time */
			return SYSINFO_RET_OK;
		}

		SET_MSG_RESULT(result, IPCS_EINVALOPT);
		return SYSINFO_RET_FAIL;
	}

	if (0 == strcmp(mode, "nsems"))
	{
		if (NULL != option)
		{
			SET_MSG_RESULT(result, IPCS_ENUMPARAM);
			return SYSINFO_RET_FAIL;
		}

		SET_UI64_RESULT(result, semid_ds.sem_nsems);	/* No. of semaphores in set */
		return SYSINFO_RET_OK;
	}

	if (0 == strcmp(mode, "ncount"))
		return ipcs_semaphore_loop(semid, semid_ds.sem_nsems, GETNCNT, option, result);

	if (0 == strcmp(mode, "zcount"))
		return ipcs_semaphore_loop(semid, semid_ds.sem_nsems, GETZCNT, option, result);

	SET_MSG_RESULT(result, IPCS_EINVALMOD);
	return SYSINFO_RET_FAIL;
}

/* some more bureaucracy */

ZBX_METRIC	*zbx_module_item_list(void)
{
	/* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
	static ZBX_METRIC	keys[] =
	/*	KEY				FLAG		FUNCTION			TEST PARAMETERS */
	{
		{"ipcs-shmem-discovery",	CF_HAVEPARAMS,	ipcs_shmem_discovery,		NULL},
		{"ipcs-shmem-details",		CF_HAVEPARAMS,	ipcs_shmem_details,		"0,size"},
		//TODO shmem limits
		//TODO shmem summary

		{"ipcs-queue-discovery",	CF_HAVEPARAMS,	ipcs_queue_discovery,		NULL},
		{"ipcs-queue-details",		CF_HAVEPARAMS,	ipcs_queue_details,		"0,permissions"},
		//TODO queue limits
		//TODO queue summary

		{"ipcs-semaphore-discovery",	CF_HAVEPARAMS,	ipcs_semaphore_discovery,	NULL},
		{"ipcs-semaphore-details",	CF_HAVEPARAMS,	ipcs_semaphore_details,		"0,nsems"},
		//TODO semaphore limits
		//TODO semaphore summary

		{NULL}
	};

	return keys;
}
