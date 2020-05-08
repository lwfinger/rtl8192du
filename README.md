rtl8192du
=========

Source code for RTL8192DU device

Install Instructions: 
    sudo dnf install -y dkms git gcc gcc-c++ kernel-headers kernel-devel make 
    git clone https://github.com/lwfinger/rtl8192du.git
    cd rtl8192du
    make
    sudo make install
    sudo modprobe rtl8192du
    
