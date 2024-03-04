
for i in $(git remote); do
    git pull "$i" master
done
