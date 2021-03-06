FROM ubuntu:14.04
MAINTAINER Albert Vilella, avilella@gmail.com

ENV INSTALL_DIR /opt/avilella
ENV APP variscan

ENV PACKAGES wget binutils make csh g++ sed gawk perl dh-autoreconf
RUN apt-get update -y && apt-get install -y --no-install-recommends ${PACKAGES}

ADD t ${INSTALL_DIR}/${APP}/t	       
ADD src ${INSTALL_DIR}/${APP}/src	       
ADD doc ${INSTALL_DIR}/${APP}/doc	       
ADD GUI ${INSTALL_DIR}/${APP}/GUI	       
ADD bin ${INSTALL_DIR}/${APP}/bin	       
ADD data ${INSTALL_DIR}/${APP}/data	       
ADD scripts ${INSTALL_DIR}/${APP}/scripts	       
ADD LastWave_2_0_4 ${INSTALL_DIR}/${APP}/LastWave_2_0_4 

COPY ChangeLog ${INSTALL_DIR}/${APP}/

COPY configure.ac ${INSTALL_DIR}/${APP}/
COPY Makefile.am ${INSTALL_DIR}/${APP}/
COPY variscan.sh ${INSTALL_DIR}/${APP}/

COPY COPYING ${INSTALL_DIR}/${APP}/
COPY NEWS ${INSTALL_DIR}/${APP}/
COPY README ${INSTALL_DIR}/${APP}/
COPY AUTHORS ${INSTALL_DIR}/${APP}/
COPY INSTALL ${INSTALL_DIR}/${APP}/

WORKDIR ${INSTALL_DIR}/${APP} 
RUN autoreconf --install &&\
#    aclocal ;\
#    autoconf ;\
#    automake --add-missing ;\
#    ./configure --prefix=${INSTALL_DIR}/${APP}/build &&\
    ./configure &&\
    make &&\
    make install

# Define an entrypoint

ENTRYPOINT ["/opt/avilella/variscan/build/bin/variscan","/opt/avilella/variscan/data/hapmap_example.hapmap","/opt/avilella/variscan/data/hapmap_example.conf"]
