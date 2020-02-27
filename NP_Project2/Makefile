# SUBDIRS = np_simple_/. #  np_single_proc_/. np_multi_proc_/.

SUBDIRS = np_single_proc_/.

all: $(SUBDIRS)

.PHONY: $(SUBDIRS) 

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: $(SUBDIRS) 
