# Install Grafana using Ansible playbooks

## Prerequisite

Setup `ansible.cfg` to point to the correct `inventory.ini`

Edit the `inventory.ini` to use proper ipn addresses

## Install Grafana

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/grafana

# Run playbook
ansible-playbook pb-install-grafana-server-x86_64.yml
```
## Setup data source

Using `prometheus` data source

## Un-install Grafana

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/grafana

# Run playbook
ansible-playbook pb-remove-grafana-server-x86_64.yml
```
