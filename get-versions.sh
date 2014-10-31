#!/bin/sh

# Script to specify versions of tools to use.

# Copyright (C) 2012, 2013 Synopsys Inc.
# Copyright (C) 2014, Embecosm Limited.

# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

# This script is sourced to specify the versions of tools to be built.

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
# Usage:

#     get-versions.sh <basedir> <componentfile> <logfile>
#                     [--auto-checkout | --no-auto-checkout]
#                     [--auto-pull | --no-auto-pull]

# We checkout the desired branch for each tool, but only if it is a git
# repository.


################################################################################
#                                                                              #
#			       Shell functions                                 #
#                                                                              #
################################################################################

# Function to check for relative directory and makes it absolute

# @param[in] $1  The directory to make absolute if necessary.
absdir() {
    case ${1} in

	/*)
	    echo "${1}"
	    ;;

	*)
	    echo "${PWD}/${1}"
	    ;;
    esac
}


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



################################################################################
#                                                                              #
#			       Parse arguments                                 #
#                                                                              #
################################################################################

# Default options
basedir=$1
shift
componentfile=$1
shift
logfile=$1
shift
autocheckout="--auto-checkout"
autopull="--auto-pull"

# Parse options
until
opt=$1
case ${opt} in
    --auto-checkout | --no-auto-checkout)
	autocheckout=$1
	;;

    --auto-pull | --no-auto-pull)
	autopull=$1
	;;

    ?*)
	echo "Usage: get-versions.sh  [--auto-checkout | --no-auto-checkout]"
        echo "                        [--auto-pull | --no-auto-pull]"
	exit 1
	;;

    *)
	;;
esac
[ "x${opt}" = "x" ]
do
    shift
done


################################################################################
#                                                                              #
#		      Check out the version of each file                       #
#                                                                              #
################################################################################

# We have to deal with some awkward cases here, because we have to deal with
# the possibility that we may currently be on a detached HEAD (so cannot
# fetch), or we will to checkout a detached HEAD (e.g. a tag). We also need to
# deal with the case that the branch we wish to checkout is not yet in the
# local repo, so we need to fetch before checking out.

# The particularly awkward case is when we are detached, and want to checkout
# a branch which is not yet in the local repo. In this case we must checkout
# some other branch, then fetch, then checkout the branch we want. This has a
# performance penalty, but only when coming from a detached branch.

# In summary the steps are:
# 1. If we are in detached HEAD state, checkout some arbitrary branch.
# 2. Fetch (in case new branch)
# 3. Checkout the branch
# 4. Pull unless we are in a detached HEAD state.

# Steps 1, 2 and 4 are only used if we have --auto-pull enabled.

# All this will go horribly wrong if you leave uncommitted changes lying
# around or if you change the remote. Nothing then but to sort it out by hand!

OLD_IFS=${IFS}
IFS="
"

# Keep trying all the repos, and then return a combined status. It's a pain in
# the neck not to find all the repo problems in one run.
status="0"

for line in `cat ${basedir}/${componentfile} | grep -v '^#' \
                 | grep -v '^$'`
do
    tool=`echo ${line} | cut -d ':' -f 1`
    branch=`echo ${line} | cut -d ':' -f 2`

    # Select the tool dir
    if ! cd ${basedir}/${tool}
    then
	logterm "ERROR (versions): No component directory for ${tool}"
	status="1"
	continue
    fi

    # Ignore tools that are not in git repos. If you have renamed .git or have
    # a non-git repo with a directory of that name, we presume you are an
    # expert who ca sort out the ensuing chaos!
    if ! [ -d ".git" ]
    then
	continue
    fi

    if [ "x${autopull}" = "x--auto-pull" ]
    then
	logterm "Fetching ${tool}"

	# See note below why two expressions are required.
	if git branch | grep -q -e '\* (detached from .*)' -e '\* (no branch)'
	then
	    # Try to guess a good branch with a working tree similar to the
	    # tag's
	    if [ "x${origbranch}" != "x${branch}" ]
	    then
		git checkout ${origbranch} >> ${logfile} 2>&1 || true
	    fi

	    # Check if repo is still in detached head
	    if git branch | grep -q -e '\* (detached from .*)' -e '\* (no branch)'
	    then
		# Detached head. Checkout an 'arbitrary' branch...
		# ... but it cannot be a remote name, or HEAD etc. ...
		arb_br=$(basename $(git branch -a | sed 's,^[ ]*,,g' \
		    | cut -f1 -d " " | grep -v '^\*' | grep -v HEAD | head -1 ))
		logonly "Note: detached HEAD, interim checkout of ${arb_br}"
		if ! git checkout ${arb_br} >> ${logfile} 2>&1
		then
		    logterm "ERROR (versions): Failed interim checkout"
		    status="1"
		    continue
		fi
	    fi
	fi

	# Fetch any new branches
	if ! git fetch >> ${logfile} 2>&1
	then
	    logterm "ERROR (versions): Failed to fetch ${tool}"
	    status="1"
	    continue
	fi

	# Fetch any new tags
	logonly "  fetching tags"
	if ! git fetch --tags >> ${logfile} 2>&1
	then
	    logterm "ERROR (versions): Failed to fetch tags for ${tool}"
	    status="1"
	    continue
	fi
    fi

    if [ "x${autocheckout}" = "x--auto-checkout" ]
    then
	logterm "Checking out ${branch}"
	if ! git checkout ${branch} >> ${logfile} 2>&1
	then
	    logterm "ERROR (versions): Failed to checkout ${branch} of ${tool}"
	    status="1"
	    continue
	fi
    fi

    if [ "x${autopull}" = "x--auto-pull" ]
    then
        # Only update to latest if we are not in detached HEAD mode.
        # If tree is in detahed state, output differs between Git versions:
        # Git 1.8 prints: * (detached from <tag_name>>)
        # Git <1.8 prints: * (no branch)
        if ! git branch | grep -q -e '\* (detached from .*)' -e '\* (no branch)'
        then
            logterm "Pulling latest version of ${branch}"
            if ! git pull >> ${logfile} 2>&1
            then
		logterm "ERROR (versions): Failed to pull ${branch} of ${tool}"
		status="1"
		continue
            fi
        fi
    fi
done

exit ${status}
