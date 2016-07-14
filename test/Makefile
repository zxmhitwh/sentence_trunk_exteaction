
WORKROOT = ./utlis_lib
PUBLIC = $(WORKROOT)/public
NLP = $(WORKROOT)
LIB2 = $(WORKROOT)/lib2-64
LIBNAME = qa
THIRD = $(WORKROOT)/third-64

SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include
OUTPUT_DIR = output
LIB_DIR = lib
TEST_DIR = test

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS = $(patsubst %.cpp,%,$(TEST_SOURCES))

		  # -I $(NLP)/pattern_parser/include/ 
#-----------------------------------------------------------------------#
INCLUDEFLAGS = -I./$(INCLUDE_DIR) \
		   -I $(WORKROOT)/zhuyin/include/\
		   -I $(WORKROOT)/mysql/include/\
			-I$(NLP)/hci/parsing/dep-parser/include \
			-I$(LIB2)/ml/linear-models/include \
			-I$(LIB2)/ml/commonlib/include \
			-I$(NLP)/hci/crf-postag/include \
			-I$(LIB2)/wordseg \
			-I$(LIB2)/libcrf/include \
			-I$(LIB2)/others-ex/include \
			-I$(LIB2)/bsl/include \
			-I$(LIB2)/ml/hmm/include \
			-I$(PUBLIC)/odict \
			-I./glog/include \
			-I ./mt_seg/mtseg/include \
			-I ./mt_seg/ltp_nlp_tools/include  \
			-I$(PUBLIC)/sign/output/include \
			-I$(LIB2)/dict/include \
			-I$(LIB2)/ullib/include \
			-I$(LIB2)/ccode/include \
			-I$(THIRD)/pcre/include \
	       -I$(WORKROOT)/triedict/include/\
	       -I$(THIRD)/json-cpp/include/\

#-L $(NLP)/pattern_parser/lib -lpattern_parser 
LIBFLAGS =	-L./$(LIB_DIR)/ \
			-L$(WORKROOT)/zhuyin/lib/ -lzhuyin\
			-L$(WORKROOT)/mysql/lib/ -lmysqlclient\
			-L$(NLP)/hci/parsing/dep-parser/lib -lSynParser \
			-L$(LIB2)/ml/linear-models/lib -llinear_models \
			-L$(LIB2)/ml/commonlib/lib -lmlcommon \
			-L$(NLP)/hci/crf-postag/lib -lpostag_crf \
			-L$(LIB2)/wordseg -lsegment \
			-L$(LIB2)/libcrf/lib -lcrf \
			-L$(LIB2)/others-ex/lib -lullib_ex \
			-L$(LIB2)/ccode/lib -lulccode \
			-L$(LIB2)/bsl/lib -lbsl \
			-L$(LIB2)/ml/hmm/lib -lhmm_model \
			-L$(PUBLIC)/odict -lodict \
			-L$(PUBLIC)/sign/output/lib -luln_sign \
			-L$(LIB2)/dict/lib -luldict \
			-L$(LIB2)/ullib/lib -lullib \
			-L$(THIRD)/pcre/lib -lpcre \
            -L$(WORKROOT)/triedict/lib -ltdict \
            -L$(THIRD)/json-cpp/lib -ljson \
			-L ./mt_seg/mtseg/lib -lsegmentor \
			-L ./mt_seg/ltp_nlp_tools/libs -lnlp_parser \
			-lboost_date_time \
			-lpthread\
			-lrt \
			-lm \
			-lz


CXXFLAGS = -g -DMUDUO_STD_STRING -DHAVE_NETINET_IN_H -DMUDUO_LOG -Wno-deprecated
#CXXFLAGS = -g -Wall -W -pipe -fPIC
#CXXFLAGS = -g -Wall -DMUDUO_STD_STRING -DHAVE_NETINET_IN_H -DMUDUO_LOG
CXX = g++ $(CXXFLAGS)

.PHONY: all test lib release clean

#-----------------------------------------------------------------------#
# make all                                                              #
#-----------------------------------------------------------------------#
all : lib test output

#-----------------------------------------------------------------------#
# make lib                                                              #
#-----------------------------------------------------------------------#
lib : $(OBJS)
	if [ ! -d $(LIB_DIR) ]; then mkdir $(LIB_DIR); fi
	ar -ruv $(LIB_DIR)/lib$(LIBNAME).a $(OBJS)

$(OBJS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(INCLUDEFLAGS)

output : 
	if [ ! -d $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi
	cp -r $(INCLUDE_DIR) $(OUTPUT_DIR)
	cp -r $(LIB_DIR) $(OUTPUT_DIR)
	cp -r $(BIN_DIR) $(OUTPUT_DIR)
	mkdir -r ../output/bin/
	cp bin/kefu_mserver ../output/bin/
	cp -r data ../output/
	cp -r rule ../output/
	cp -r dict ../output/

#-----------------------------------------------------------------------#
# make test                                                             #
#-----------------------------------------------------------------------#
test: $(TEST_BINS)

$(TEST_BINS): %: %.cpp
	if [ ! -d $(BIN_DIR) ]; then mkdir $(BIN_DIR); fi
	$(CXX) $< -o $@ $(INCLUDEFLAGS) -l$(LIBNAME) $(LIBFLAGS)
	mv $@ $(BIN_DIR)/

clean :
	rm -rf $(LIB_DIR)/lib$(LIBNAME).a
	rm -rf $(OBJS)
	rm -rf $(SRC_DIR)/*.o $(SRC_DIR)/*~
	rm -f $(BIN_DIR)/*
