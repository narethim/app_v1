# Test Ansible connectivity using Ansible playbooks

## Prerequisite

Setup `ansible.cfg` to point to the correct `inventory.ini`

Edit the `inventory.ini` to use proper ipn addresses

## Test connectivity

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/test

# Run playbooks
ansible-playbook pb-test-connection.yml

ansible-playbook pb-test-connection2.yml

```
