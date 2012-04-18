SUBDIRS =

if BUILD_DEMOUI
SUBDIRS += c-demo
endif

#### Funny OR simulation :)
if BUILD_TESTS
if !BUILD_UNITTESTS
SUBDIRS += shared
endif
endif

if BUILD_UNITTESTS
if !BUILD_TESTS
SUBDIRS += shared
endif
endif

if BUILD_UNITTESTS
if BUILD_TESTS
SUBDIRS += shared
endif
endif
### 

if BUILD_TESTS
SUBDIRS += memory functional 
endif
