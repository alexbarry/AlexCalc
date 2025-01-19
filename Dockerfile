FROM nginx:latest AS base

ARG ALEXCALC_BUILD_TYPE_LABEL
ENV ALEXCALC_BUILD_TYPE_LABEL=${ALEXCALC_BUILD_TYPE_LABEL}

# Install OS dependencies
RUN apt-get update && apt-get install -y \
	cmake \
	python3 \
	xz-utils \
	npm \
	git

# Install Emscripten
WORKDIR /app
RUN if [ ! -f "emsdk" ]; then \
	echo "Cloning emsdk repo..."; \
	git clone https://github.com/emscripten-core/emsdk.git emsdk ; \
else \
	echo "emsdk repo already present"; \
fi

WORKDIR /app/emsdk
RUN ./emsdk install latest
RUN ./emsdk activate latest
RUN . ./emsdk_env.sh
ENV PATH="${PATH}:/app/emsdk"
#ENV EM_CONFIG="/root/.emscripten"
RUN /bin/bash -c "source ./emsdk_env.sh"
ENV PATH="${PATH}:/app/emsdk/upstream/emscripten"
RUN emcc --version

RUN npm -g install typescript


# Build AlexCalc
WORKDIR /app
COPY . .
WORKDIR build/wasm/
RUN bash ./build.sh

# Copy build output to nginx serving path
# TODO only copy html/css/js/wasm/png?
RUN cp -r out/* /usr/share/nginx/html/

FROM scratch AS export_output
COPY --from=base /app/build/wasm/out/*.html   /
COPY --from=base /app/build/wasm/out/*.png    /
COPY --from=base /app/build/wasm/out/*.txt    /
COPY --from=base /app/build/wasm/out/js       /js/
COPY --from=base /app/build/wasm/out/css      /css/
COPY --from=base /app/build/wasm/out/graphics /graphics/

FROM base AS server
# Use base nginx entrypoint to host HTTP server
