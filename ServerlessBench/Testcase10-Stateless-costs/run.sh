cd action
javac -cp .:gson-2.8.2.jar JavaResize.java
./action_update.sh

cd ../test
./eval.sh
