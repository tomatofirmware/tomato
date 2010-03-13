deps_config := \
	extra/Configs/Config.in \
	extra/Configs/Config.in.arch \
	extra/Configs/Config.mipsel

.config include/bits/uClibc_config.h: $(deps_config)

$(deps_config):
