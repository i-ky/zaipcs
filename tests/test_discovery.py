from json import loads
from zabbix import get

def test_shmem():
    for segment in loads(get("ipcs-shmem-discovery"))["data"]:
        assert "{#KEY}" in segment
        id = segment["{#ID}"]
        assert segment["{#OWNER}"] == get(f"ipcs-shmem-details[{id},owner,uid]")
        assert int(segment["{#PERMS}"], base=8) == int(get(f"ipcs-shmem-details[{id},permissions]"))

def test_queue():
    for queue in loads(get("ipcs-queue-discovery"))["data"]:
        assert "{#KEY}" in queue
        id = queue["{#ID}"]
        assert queue["{#OWNER}"] == get(f"ipcs-queue-details[{id},owner,uid]")
        assert int(queue["{#PERMS}"], base=8) == int(get(f"ipcs-queue-details[{id},permissions]"))

def test_semaphore():
    for semaphore in loads(get("ipcs-semaphore-discovery"))["data"]:
        assert "{#KEY}" in semaphore
        id = semaphore["{#ID}"]
        assert semaphore["{#OWNER}"] == get(f"ipcs-semaphore-details[{id},owner,uid]")
        assert int(semaphore["{#PERMS}"], base=8) == int(get(f"ipcs-semaphore-details[{id},permissions]"))
