if [ -z $1 ]; then
	dir="Data/*"
else
	dir=$1
fi

if [ -n $2 ]; then
	vertex=$2
fi

for i in $dir/*.obj; do
	./gaussMap $i $vertex
done;
