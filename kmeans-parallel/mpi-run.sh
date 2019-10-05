#bash
proc_num="invalid"

while [ `echo $proc_num | grep "[^0-9]" > /dev/null; echo $?` = "0" ]; do
	clear
	echo =======================================================
	echo ======================= k-means =======================
	echo =======================================================
	echo Enter number of processors:
	read proc_num
done

echo
echo Updating XML files...
cp ../kmeans-preprocess/code-list.xml ./code-list.xml
cp ../kmeans-preprocess/term-collection.xml ./term-collection.xml
echo Executing in parallel with $proc_num processors...
echo
echo

if [ "$2" = "" ]; then
	mpirun -np $proc_num $1
else
	mpirun -dbg=pgdbg -np $proc_num $1
fi
read buffer
