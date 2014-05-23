#!/usr/bin/env bash

##############################################################################
##
## Install and configure Jenkins Continuous Integration Tool for building
## the Epiphany toolchain and SDK on a nightly basis.
##
## References:
##
##     https://wiki.jenkins-ci.org/display/JENKINS/Installing+Jenkins+on+Ubuntu
##

install_jenkins()
{
	local key_uri=http://pkg.jenkins-ci.org/debian/jenkins-ci.org.key
	printf "=================================================================\n"
	printf "==       Installing Jenkins Continuous Integration Tool        ==\n"
	printf "=================================================================\n"

	if ! wget -q -O - $key_uri | sudo apt-key add - ; then
		printf "Failed to download the Jenkins key from $key_uri\n"
		exit 1
	fi

	sudo sh -c 'echo deb http://pkg.jenkins-ci.org/debian binary/ > /etc/apt/sources.list.d/jenkins.list'
	sudo apt-get -y update &> /dev/null
	sudo apt-get -y install jenkins	&> /dev/null
}

configure_epiphany_build()
{
	printf "=================================================================\n"
	printf "==           Configuring Jenkins Epiphany-SDK build            ==\n"
	printf "=================================================================\n"

	sudo tar xz -C ${jenkins_home}/jobs/ -f ./epiphany_build_jenkins.tar.gz

	sudo chown -R jenkins:jenkins ${jenkins_home}
}

restart_jenkins_service()
{
	sudo service jenkins restart
}

install_epiphany_build_scripts()
{
	sudo mkdir -p ${jenkins_home}/epiphany_build
	sudo cp ./build_scripts/*.sh ${jenkins_home}/epiphany_build/

	sudo chown -R jenkins:jenkins ${jenkins_home}/epiphany_build
}

jenkins_home=/var/lib/jenkins

install_jenkins
restart_jenkins_service
install_epiphany_build_scripts
configure_epiphany_build
