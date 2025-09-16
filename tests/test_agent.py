from zabbix import get

def test_ping():
    assert get("agent.ping") == "1"
