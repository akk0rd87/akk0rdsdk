
for i in $(git remote); do
    git push "$i" master
done
