# Install Grafana using Ansible playbooks

## Prerequisite

Setup `ansible.cfg` to point to the correct `inventory.ini`

Edit the `ns3-host.ini` to use proper ip addresses

## Running ns-3 playbooks

```sh
# Change current dir to working dir
cd ~/projects/git_tmp/app_v1/ansible/playbooks/ns3

# Run playbooks
ansible-playbook pb-ns3-run-test-apps.yml

# Run playbooks using waf and loop
ansible-playbook pb-waf.yml

ansible-playbook pb-waf-loop.yml

ansible-playbook pb-waf-loop-25.yml

ansible-playbook pb-waf-loop-30.yml

ansible-playbook pb-waf-loop-35.yml

ansible-playbook pb-waf-loop-40.yml

```

