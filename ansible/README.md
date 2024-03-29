# Checkout ansible

## 1. Checkout ns-3 connectivity

```sh
cd ~/projects/git_tmp/app_v1/ansible/playbooks/test
ansible-playbook pb-test-connection.yml
```

```sh
ubuntu@ubuntu-node2:~/projects/git_tmp/app_v1/ansible/playbooks/test$ ansible-playbook pb-test-connection.yml

PLAY [test_node] ***********************************************************************************************************************

TASK [Gathering Facts] *****************************************************************************************************************
ok: [10.161.6.161]

TASK [ping] ****************************************************************************************************************************
ok: [10.161.6.161]

TASK [debug] ***************************************************************************************************************************
ok: [10.161.6.161] => {
    "ansible_distribution": "Ubuntu"
}

PLAY [test_node] ***********************************************************************************************************************

TASK [Gathering Facts] *****************************************************************************************************************
ok: [10.161.6.161]

TASK [test connection with 'ping' module] **********************************************************************************************
ok: [10.161.6.161]

TASK [debug 'var=ansible_distribution'] ************************************************************************************************
ok: [10.161.6.161] => {
    "ansible_distribution": "Ubuntu"
}

PLAY RECAP *****************************************************************************************************************************
10.161.6.161               : ok=6    changed=0    unreachable=0    failed=0    skipped=0    rescued=0    ignored=0
```

```sh
cd ~/projects/git_tmp/app_v1/ansible/playbooks/test
ansible-playbook pb-test-connection2.yml
```

Example:

```sh
ubuntu@ubuntu-node2:~/projects/git_tmp/app_v1/ansible/playbooks/test$ ansible-playbook pb-test-connection2.yml

PLAY [test_node] ***********************************************************************************************************************

TASK [Gathering Facts] *****************************************************************************************************************
ok: [10.161.6.161]

TASK [test connection with 'ping' module] **********************************************************************************************
ok: [10.161.6.161]

TASK [debug 'var=ansible_distribution'] ************************************************************************************************
ok: [10.161.6.161] => {
    "ansible_distribution": "Ubuntu"
}

TASK [ping] ****************************************************************************************************************************
ok: [10.161.6.161]

TASK [debug] ***************************************************************************************************************************
ok: [10.161.6.161] => {
    "ansible_distribution": "Ubuntu"
}

PLAY RECAP *****************************************************************************************************************************
10.161.6.161               : ok=5    changed=0    unreachable=0    failed=0    skipped=0    rescued=0    ignored=0
```
