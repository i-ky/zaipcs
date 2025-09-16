from os.path import abspath, dirname
from subprocess import check_output

def get(key):
    return check_output(
        ["docker-compose", "exec", "-T", "agent", "zabbix_get", "-s", "localhost", "-k", key],
        cwd = dirname(abspath(__file__))
    ).decode().rstrip()
