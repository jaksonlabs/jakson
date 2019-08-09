Taken from [https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/](https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)

```
sudo apt-get install libgtest-dev

sudo apt-get install cmake # install cmake

cd /usr/src/gtest

sudo cmake CMakeLists.txt

sudo make

sudo cp *.a /usr/lib

sudo mkdir /usr/local/lib/gtest

sudo ln -s /usr/lib/libgtest.a /usr/local/lib/gtest/libgtest.a

sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/gtest/libgtest_main.a
```