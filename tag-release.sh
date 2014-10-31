#!/bin/sh

# Script to specify versions of tools to use.

# Copyright (C) 2012-2014 Synopsys Inc.
# Copyright (C) 2014 Embecosm Limited.

# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
# Contributor Anton Kolesov <Anton.Kolesov@synopsys.com>

# This script is used to tag a particular release.

# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.

# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

# -----------------------------------------------------------------------------

# Tag all the current HEADs of releases for testing.

# Usage:

#     ./tag-release.sh <tagname> <toolchain_components>

# For each repository, the branch chosen by get-versions.sh will be tagged,
# and that branch MUST have an associated upstream.

# This does the following steps

#  1. Run get-versions.sh to checkout the heads of all component trees

#  2. Tags and pushes the tag for all the component trees *except* toolchain.

#  3. Edits toolchain-components so it checks out the tagged versions of all
#     components.

#  4. Commits this change, tags that commit and pushes that tag.

# At the end, the SDK branch will be checked out on that tag. For
# ongoing development we'll need to checkout the release branch and modify the
# component file as necessary.

################################################################################
#                                                                              #
#			       Shell functions                                 #
#                                                                              #
################################################################################

# Convenience function to copy a message to the log and terminal

# @param[in] $1  The message to log
logterm () {
    echo $1 | tee -a ${logfile}
}


# Convenience function to copy a message to the log only

# @param[in] $1  The message to log
logonly () {
    echo $1 >> ${logfile}
}


# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

# Set the release parameters
. ${basedir}/sdk/define-release.sh

# Get the arguments
if [ $# != 2 ]
then
    echo "Usage: ./tag-release.sh <tagname> <component-file>"
    exit 1
else
    tagname=$1
    componentfile=$2
fi

# Set up a clean log
logfile=${LOGDIR}/tagging-$(date -u +%F-%H%M).log
rm -f "${logfile}"
echo "Logging to ${logfile}"

# Make sure we are up to date. It is possible we are detached, so pull will
# fail, but that doesn't matter.
logterm "Pulling SDK repo if possible"
git pull > /dev/null 2>&1 || true

# Check out heads of component trees
logterm "Checking out all repos..."
if ! ${basedir}/sdk/get-versions.sh ${basedir} sdk/${componentfile} \
                                    ${logfile} --auto-pull \
                                    --auto-checkout
then
    logterm "ERROR: Failed to check out component repos."
    exit 1
fi
logterm "All repos checked out"

# Sanity check that each branch has a remote
# All this will go horribly wrong if you leave uncommitted changes lying
# around or if you change the remote. Nothing then but to sort it out by hand!

OLD_IFS=${IFS}
IFS="
"

tool_list=""
for line in `cat ${basedir}/sdk/${componentfile} | grep -v '^#' \
                 | grep -v '^$'`
do
    tool=`echo ${line} | cut -d ':' -f 1`
    tool_list="${tool} ${tool_list}"

    # Select the tool dir
    if ! cd ${basedir}/${tool}
    then
	logterm "ERROR: No component directory for ${tool}"
	exit 1
    fi

    if ! branch=`git symbolic-ref -q HEAD --short`
    then
	logterm "ERROR: ${tool} is in detached head mode"
	exit 1
    fi

    if ! remote=`git config branch.${branch}.remote`
    then
	logterm "ERROR: branch ${branch} of ${tool} has no upstream"
	exit 1
    fi
done

IFS=${OLD_IFS}

# Tag and push the tags for each component
for tool in ${tool_list}
do
    logterm "Tagging ${tool}..."
    cd ${basedir}/${tool} > /dev/null 2>&1
    branch=`git symbolic-ref -q HEAD --short`
    remote=`git config branch.${branch}.remote`

    # The tagname includes the tool name, otherwise we get a clash in the
    # binutils-gdb repo.  The full tag name is epiphany-<tool>-<tagname>
    full_tagname="epiphany-${tool}-${tagname}"
    if git tag -f ${full_tagname}
    then
	logterm "Tagged ${tool} with ${full_tagname}"
    else
	logterm "ERROR: Failed to tag ${tool}"
	exit 1
    fi

    if ! git push ${remote} ${full_tagname}
    then
	logterm "ERROR: Failed to push tag for ${tool}"
	exit 1
    fi
done

# Get the remote for the current SDK branch
cd ${basedir}/sdk
branch=`git symbolic-ref -q HEAD --short`
remote=`git config branch.${branch}.remote`

# Edit componentfile
logterm "Editing ${componentfile}..."
for tool in ${tool_list}
do
    full_tagname="epiphany-${tool}-${tagname}"
    if ! sed -i -e "s/^\(${tool}\):[^:]*:/\1:${full_tagname}:/" \
	${componentfile}
    then
	logterm "ERROR: Failed to edit ${component_file} for ${tool}"
	exit 1
    fi
done

logterm "Committing ${componentfile}..."
if ! git commit -a -m "Create ${componentfile} for tag ${tagname}"
then
    logterm "ERROR: Failed to commit ${componentfile}"
    exit 1
fi

# Tag and push the commit
logterm "Tagging SDK"
if ! git tag -f ${tagname}
then
    logterm "ERROR: Failed to tag SDK"
    exit 1
fi

if ! git push ${remote} ${tagname}
then
    logterm "ERROR: Failed to push tag for SDK"
    exit 1
fi

logterm "All repositories tagged as ${tagname}"
