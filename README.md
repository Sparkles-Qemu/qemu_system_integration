# qemu_system_integration
This baseline project is the starting point that you can use for the tutorial. Please use Vscode for this because it'll simplify development significantly. Stick to remote.coe.neu.edu too to prevent COE from issuing an unecessary ban hammer. The Makefile allows building/ running the simulation with make and make run. Vscode also has some convenient shortcuts with Cmd+shift+b and Cmd+shift+d (for mac) and ctrl+shift+b and ctrl+shift+d (for windows/ linux I assume). A lot of comments were added to the base project to explain some of SystemC's idiosyncrasys. The tutorial can be found here: http://www.asic-world.com/systemc/tutorial.html. 
Please do not commit changes on main branch. Either develop locally or push to your own git branch. 

Clone the repo after ssh-ing into remote.coe.neu.edu then connect with vscode to start developing.
(It might be useful to update your ssh config files as described below before starying vscode)

# GTKwave usage
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

3) In the integrated terminal go to the src directory and run:
make gtkwave WAVE=/traces/sim_signals.trace.vcd

Please note that sim_signals.trace.vcd is just a signal file, so if you are
planning on generating other wave files you can change the WAVE variable. 

Good luck guys! 

Aly
