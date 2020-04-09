# Description
This baseline project is the starting point that you can use for the tutorial. Please use Vscode for this because it'll simplify development significantly. Stick to remote.coe.neu.edu too to prevent COE from issuing an unecessary ban hammer. The Makefile allows building/ running the simulation with make and make run. Vscode also has some convenient shortcuts with Cmd+shift+b and Cmd+shift+d (for mac) and ctrl+shift+b and ctrl+shift+d (for windows/ linux I assume). A lot of comments were added to the base project to explain some of SystemC's idiosyncrasys. The tutorial can be found here: http://www.asic-world.com/systemc/tutorial.html. 
Please do not commit changes on main branch. Either develop locally or push to your own git branch. 

Clone the repo after ssh-ing into remote.coe.neu.edu then connect with vscode to start developing.
(It might be useful to update your ssh config files as described below before starying vscode)

# GTKwave usage in vscode
Gtkwave usage has been added, to set it up in vscode do the following:

1) Install the extention:
   https://marketplace.visualstudio.com/items?itemName=spadin.remote-x11-ssh or
   search in vscode for Remote X11(SSH)

2) Add the following to your ssh config file (~/.ssh/config), change user field
   to match your own username

        Host gateway

         User //TODO: Update coe username

         Hostname gateway.coe.neu.edu

        Host remote

         User //TODO: Update coe username

         ProxyCommand ssh -X -W %h:%p gateway

         Hostname remote.coe.neu.edu

3) In the integrated terminal go to the project src directory and run:
make gtkwave WAVE=/traces/sim_signals.trace.vcd

# GTKwave usage outside of vscode
If GTKWave doesn't launch from within vscode you can just use a regular terminal.
1) ssh remote
2) cd to project_directory/src
3) run: make gtkwave WAVE=/traces/sim_signals.trace.vcd

Please note that sim_signals.trace.vcd is just a signal file, so if you are
planning on generating other wave files you can change the WAVE variable. 

# QEMU+SystemC simulation access
1) Ask bruno for an account on Diabase (server in ESL lab)
2) ssh into peta@172.17.0.2 with password 123456789
3) Start tmux or ssh twice
4) cd ~/qemu_zcu102 and run ./launch.sh
5) In screen cd to ~/systemctlm-cosim-demo and run ./launch
6) qemu should boot into linux with access to some registers exposed by systemc

Relevant resources:
1) Slides on Systemc and SystemcTLM http://users.ece.utexas.edu/~gerstl/ee382m_f18/lectures/lecture_3.pdf

2) Setup process for Qemu+SystemC simulating a ZynqMP http://users.ece.utexas.edu/~gerstl/ee382m_f18/labs/QEMU_SystemC_Tutorial.htm

3) SystemC TLM tutorial https://www.doulos.com/knowhow/systemc/tlm2/

4) SystemC tutorial https://www.doulos.com/knowhow/systemc/tutorial/ 

5) Xilinx QEMU+SystemC setup tutorial https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842109/QEMU+SystemC+and+TLM+CoSimulation

6) Repo for Xilinx SystemC Demo https://github.com/Xilinx/systemctlm-cosim-demo

7) Repo for Xilinx libsoc https://github.com/Xilinx/libsystemctlm-soc



Aly
