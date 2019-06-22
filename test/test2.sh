# #!/bin/bash
wmctrl -s 2 #Switches to workspace 0 [workspaces are numbered from 0]

TERMINAL_SIZE=83x17

P1=000+000
P2=683+000
P3=000+400
P4=683+400

gnome-terminal --geometry $TERMINAL_SIZE+$P1 -e "bash -ic 'bin/./server 5000 1 localhost 4999 localhost 4998 localhost 4997; exec bash'"
# sleep 2
gnome-terminal --geometry $TERMINAL_SIZE+$P2 -e "bash -ic 'bin/./server 4999 0 localhost 5000 localhost 4998 localhost 4997; exec bash'"
# sleep 2
gnome-terminal --geometry $TERMINAL_SIZE+$P3 -e "bash -ic 'bin/./server 4998 0 localhost 5000 localhost 4999 localhost 4997; exec bash'"
# sleep 2
gnome-terminal --geometry $TERMINAL_SIZE+$P4 -e "bash -ic 'bin/./server 4997 0 localhost 5000 localhost 4999 localhost 4998; exec bash'"

