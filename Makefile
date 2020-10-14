# The binaries that we want to build
PGMS := bin/lists bin/section_data bin/toc_diff bin/list_issues bin/set_status
CXXFLAGS := -std=c++17 -Wall -g -O2 -D_GLIBCXX_ASSERTIONS

# Running 'make debug' is equivalent to 'make DEBUG=1'
ifeq "$(MAKECMDGOALS)" "debug"
DEBUG := 1
endif

# Running 'make DEBUG=blah' rebuilds all binaries with debug settings
ifdef DEBUG
debug: pgms
CPPFLAGS := -DDEBUG_SUPPORT -D_GLIBCXX_DEBUG
CXXFLAGS += -O0
ifdef LOGGING
CPPFLAGS += -DDEBUG_LOGGING
endif
# Add UBSAN=1 to the command line to enable Undefined Behaviour Sanitizer
ifdef UBSAN
CXXFLAGS += -fsanitize=undefined
endif
# Add ASAN=1 to the command line to enable Address Sanitizer
ifdef ASAN
CXXFLAGS += -fsanitize=address
endif
# Make 'clean' a prerequisite of all object files, so everything is rebuilt:
src/*.o: clean
.DEFAULT_GOAL: all
endif

all: pgms

pgms: $(PGMS)

bin/lists: src/date.o src/issues.o src/status.o src/sections.o src/mailing_info.o src/report_generator.o src/lists.o

bin/section_data: src/section_data.o

bin/toc_diff: src/toc_diff.o

bin/list_issues: src/date.o src/issues.o src/status.o src/sections.o src/list_issues.o

bin/set_status: src/set_status.o src/status.o

$(PGMS):
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(PGMS) src/*.o

distclean: clean
	rm -f meta-data/annex-f meta-data/networking-annex-f
	rm -f meta-data/dates
	rm -f -r mailing

history: bin/lists
	bin/lists revision history

mailing:
	mkdir $@

lists: mailing bin/lists dates
	bin/lists

define update
  if diff -N -u $(1) $(1).tmp ; then rm $(1).tmp ; else mv $(1).tmp $(1) ; fi
endef

WG21 := $(HOME)/src/cplusplus
DRAFT := $(WG21)/draft
NET := $(WG21)/networking-ts
filter-annex-f := sed 's/\\newlabel{\([^}]*\)}.*TitleReference {\([^}]*\)}.*/\1 \2/' | sed 's/\(Clause\|Annex\) //' | grep -v "aux:tab:" | grep -v "aux:fig:" | sed 's/\(.*\).aux://' | grep -v '^\\' | sort
filter-net-ts-annex-f := sed 's/\\newlabel{\([^}]*\)}.*TitleReference {\([^}]*\)}.*/\1 \2/' | sed 's/\\newlabel{\([^}]*\)}.*Clause \([^}]*\)}.*/\1 \2/' | sed 's/\\newlabel{\([^}]*\)}.*Annex \([^}]*\)}.*/\1 \2/' | grep -v "aux:tab:" | grep -v "aux:fig:" | sed 's/\(.*\).aux://' | grep -v '^\\' | sort

meta-data/annex-f: $(wildcard $(DRAFT)/source/*.aux)
	test -d "$(DRAFT)" && grep newlabel $^ | $(filter-annex-f) > $@

meta-data/networking-annex-f: $(wildcard $(NET)/src/*.aux)
	grep newlabel $^ /dev/null | $(filter-net-ts-annex-f) > $@

meta-data/networking-section.data: meta-data/networking-annex-f bin/section_data
	if [ -s $< ]; then \
	  grep '^[^[:upper:][:digit:]]' $< | grep -v ISO/IEC | bin/section_data networking.ts >> $@.tmp ; \
	  $(call update,$@) ; \
	fi

meta-data/section.data: meta-data/annex-f meta-data/networking-section.data bin/section_data
	grep '^[^[:upper:][:digit:]]' $< | grep -v ISO/IEC | bin/section_data > $@.tmp
	cat meta-data/networking-section.data >> $@.tmp
	cat meta-data/tr1_section.data >> $@.tmp
	cat meta-data/filesystem-section.data >> $@.tmp
	cat meta-data/lfts-old-section.data >> $@.tmp
	cat meta-data/lfts-v3-section.data >> $@.tmp
	$(call update,$@)

.PHONY: all pgms clean distclean lists history

dates: meta-data/dates

# Generate file with issue number and unix timestamp of last change.
meta-data/dates: xml/issue[0-9]*.xml
	for i in xml/issue[0-9]*.xml ; do \
	  n=$${i:9} ; n=$${n%.xml} ; \
	  test -e $@ && grep -q "^$$n " $@ && test $$i -ot $@ && continue ; \
	  echo $$i >&2 ; \
	  git log -1 --pretty="format:$$n %ct%n" $$i ; \
	done > $@.new
	cat $@ $@.new | sort -n -r | sort -n -k 1 -u > $@.tmp
	rm $@.new
	$(call update,$@)

.PRECIOUS: meta-data/dates

.PHONY: dates
