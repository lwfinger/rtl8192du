rtl8192du
=========

Source code for RTL8192DU device

Install Instructions:
# For Fedora
    sudo dnf install -y dkms git gcc gcc-c++ kernel-headers kernel-devel make 
# Get the source and build the driver
    git clone https://github.com/lwfinger/rtl8192du.git
    cd rtl8192du
    make
    sudo make install
    sudo modprobe 8192du
    
