CC=gcc
CFLAGS=-Wall
LDLIBS=-lwolftpm -lwolfssl -lm -pthread -lcurl

# Uncomment to enable Debug
#CFLAGS += -g -O0 -DDEBUG_PRINTS

# Define the build directory
BUILD_DIR = build

# List source files
SRCS = source/enact.c \
       source/tpm.c \
       source/tpm_update.c \
       source/ifx_fw_extract.c \
       source/ifx_fw_update.c \
       source/load_file.c \
       source/misc.c

# Generate object file names with the build directory prefix
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))

.PHONY: all
all: enact

enact: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Pattern rule to compile source files from the 'source/' directory
# into object files in the 'build/' directory
$(BUILD_DIR)/%.o: source/%.c
	@mkdir -p $(@D) # Create the build directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f enact # Remove the executable
	rm -rf $(BUILD_DIR) # Remove the entire build directory
