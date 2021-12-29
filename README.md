<pre>
rsync -avz --progress         rsync://hgdownload.cse.ucsc.edu/goldenPath/hg19/multiz46way/maf ./

for i in *.md5;do md5sum -c $i;done

cd maf
mkdir used
for i in `ls chr*.maf.gz|grep -v chrUn|grep -v random|grep -v hap`;do newname=`basename $i .gz`; gunzip -c $i > used/$newname;done

#program is hardcore as hell, so only start 2 processes, that might even be to much
ls maf/used/*.maf|xargs -P 2 -n1 -I infile sh -c './a.out infile |gzip -c >maf/info/`basename infile`.info.gz'

rm -rf used
</pre>
