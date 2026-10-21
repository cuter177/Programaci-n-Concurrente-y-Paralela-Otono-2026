set -e
cd "$(dirname "$0")"
rm -rf out
javac -d out ./*.java
java -cp out Main
