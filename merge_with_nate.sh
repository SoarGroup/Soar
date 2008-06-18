#!/bin/sh

NATE_REPO="/home/jzxu/work/soar/nlderbin-rl-epmem"
NATE_URL="https://winter.eecs.umich.edu/svn/soar/branches/nlderbin-rl-epmem"
svn merge --dry-run -r$(cat last_merge_revision):HEAD $NATE_URL
echo "Continue with merge? (y/n)"
read response
if [ $response == "y" ]; then
  svn merge -r$(cat last_merge_revision):HEAD $NATE_URL
  svn update $NATE_REPO | awk '/At revision/{print substr($3,0,length($3)-1); exit} /Updated to revision/{print substr($4,0,length($4)-1);exit}' > last_merge_revision
fi
