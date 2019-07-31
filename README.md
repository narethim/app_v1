# app_v1

app files and setup

## 1. Setup ns-3 host node

This is a RHEL 7.4 x86_64 machine. It has VirtualBox installed. An Ubuntu VM is installed to host ns-3.

## 2. Setup k3s cluster using 3 NVIDIA Jetson nodes

### 2.1 Setup k3s master node (jet_nano)

### 2.2 Setup k3s worker node 1 (jet_b)

### 2.3 Setup k3s worker node 2 (jet_j)

## 3. Setup monitoring node

This is a Ubuntu 18.04 x86_64 machine. Prometheus and Grafana are installed to monitor the network and display the host metrics.

## 4. Ansible node

Use this node to setup other nodes using ansible and its playbooks.
