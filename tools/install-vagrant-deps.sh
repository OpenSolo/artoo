sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6AF0E1940624A220
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6D1D8367A3421AFB

# fix broken  urllib3 or some ssl stuff diesn't work
sudo pip install urllib3[secure]

# fix broken python2.7.x by moving it forward to something newer than 2.7.8:
sudo add-apt-repository ppa:fkrull/deadsnakes-python2.7
sudo apt-get update
sudo apt-get install python2.7

# get deps for pillow, and note that we get Pillow from easy_install instead of 'pip' or apt-get becuase this method actually works.
sudo apt-get install python-dev python-setuptools
sudo apt-get install libtiff4-dev libjpeg8-dev zlib1g-dev libfreetype6-dev liblcms2-dev libwebp-dev tcl8.5-dev tk8.5-dev python-tk
sudo  easy_install Pillow

apt-add-repository 'deb http://ppa.launchpad.net/anatol/tup/ubuntu precise main' -y
add-apt-repository 'ppa:terry.guo/gcc-arm-embedded' -y

apt-get -qq --assume-yes update
apt-get -qq --assume-yes install tup freetype* python-pip python-dev gcc-arm-none-eabi git

sudo pip install --upgrade pip
sudo pip install pillow
