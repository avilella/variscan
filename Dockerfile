FROM debian:wheezy
MAINTAINER Albert Vilella, avilella@gmail.com

ENV INSTALL_DIR /opt/
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

ADD AUTHORS ${INSTALL_DIR}/${APP}
ADD autogen.sh ${INSTALL_DIR}/${APP}
ADD ChangeLog ${INSTALL_DIR}/${APP}

ADD configure.in ${INSTALL_DIR}/${APP}
ADD COPYING ${INSTALL_DIR}/${APP}
ADD gdb.avb ${INSTALL_DIR}/${APP}
ADD INSTALL ${INSTALL_DIR}/${APP}
ADD LICENSE ${INSTALL_DIR}/${APP}
ADD NEWS ${INSTALL_DIR}/${APP}
ADD README ${INSTALL_DIR}/${APP}
ADD README.devel ${INSTALL_DIR}/${APP}
ADD variscan.dev ${INSTALL_DIR}/${APP}
ADD variscan.sh ${INSTALL_DIR}/${APP}

RUN cd ${INSTALL_DIR} &&\
    autoreconf --install ;\
    ./configure --prefix=${INSTALL_DIR}/build &&\
    make &&\
    make install

# Define an entrypoint

ENTRYPOINT ["/opt/variscan/build/bin/variscan","/opt/variscan/data/hapmap_example.hapmap","/opt/variscan/data/hapmap_example.conf"]
