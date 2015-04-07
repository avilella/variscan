FROM debian:wheezy
MAINTAINER Albert Vilella, avilella@gmail.com

ENV INSTALL_DIR /opt/
ENV APP variscan

ENV PACKAGES wget binutils make csh g++ sed gawk perl
RUN apt-get update -y && apt-get install -y --no-install-recommends ${PACKAGES}

ADD t ${INSTALL_DIR}/${APP}/t	       
ADD src ${INSTALL_DIR}/${APP}/src	       
ADD doc ${INSTALL_DIR}/${APP}/doc	       
ADD GUI ${INSTALL_DIR}/${APP}/GUI	       
ADD bin ${INSTALL_DIR}/${APP}/bin	       
ADD data ${INSTALL_DIR}/${APP}/data	       
ADD scripts ${INSTALL_DIR}/${APP}/scripts	       
ADD LastWave_2_0_4 ${INSTALL_DIR}/${APP}/LastWave_2_0_4 

# Define an entrypoint

ENTRYPOINT ["/opt/variscan/build/bin/variscan","/opt/variscan/data/hapmap_example.hapmap","/opt/variscan/data/hapmap_example.conf"]
