sudo mkdir /dev/shm/tmp
sudo chmod 1777 /dev/shm/tmp
sudo mount --bind /dev/shm/tmp /home/hdu/Pharmacy_Robot_RAM
cp /home/hdu/Pharmacy_Robot/* /home/hdu/Pharmacy_Robot_RAM/
