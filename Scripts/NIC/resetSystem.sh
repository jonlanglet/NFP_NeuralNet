echo "Uninstalling old setup..."
sudo apt-get remove nfp-bsp nfp-bsp-dev nfp-bsp-dkms nfp-sdk

echo "Installing..."
sudo dpkg -i nfp-sdk_6.1.0.1-preview-3243-2_amd64.deb
sudo dpkg -i nfp-bsp-6000-b0_2018.06.29.1443-1_amd64.deb
cd nfp-sdk-6-rte-v6.1.0.1-preview-Ubuntu-Release-r2750-2018-10-10-ubuntu.binary/

sudo ./sdk6_rte_install.sh install

echo "Reboot now."
