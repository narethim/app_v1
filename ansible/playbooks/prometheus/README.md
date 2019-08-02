# Install Prometheus using Ansible playbooks

## Prerequisite

Setup `ansible.cfg` to point to the correct `inventory.ini`

Edit the `inventory.ini` to use proper ipn addresses

## Install Prometheus

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/Prometheus

# Run playbook
ansible-playbook pb-install-Prometheus-server-x86_64.yml
```
## Setup data source

Set `prometheus` data source

## Refresh data source

Find out the <pid> of  `prometheus` process

```sh
ps -ef | grep prometheus
```

```sh
sudo kill -s HUP <pid>
```

## Un-install Prometheus

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/Prometheus

# Run playbook
ansible-playbook pb-remove-Prometheus-server-x86_64.yml
```
