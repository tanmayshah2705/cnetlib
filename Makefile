VERSION = 1.1

.PHONY: all docs deploy_docs release clean

all: lib/libssnl.a lib/libcsnl.a

lib/libssnl.a: src/server/ssnl.o
	ar rcs lib/libssnl.a src/server/ssnl.o

lib/libcsnl.a: src/client/csnl.o
	ar rcs lib/libcsnl.a src/client/csnl.o

src/server/ssnl.o: src/server/ssnl.c include/ssnl.h
	gcc -c src/server/ssnl.c -Iinclude

src/client/csnl.o: src/client/csnl.c include/csnl.h
	gcc -c src/client/csnl.c -Iinclude


#generate documentation
docs:
	rm -rf doc/html && doxygen doc/doxyfile

#deploy generated documentation on github pages
deploy_docs: 
	./deploy-docs.sh


#create release zip
release: lib/libssnl.a lib/libcsnl.a template/server/server_template.c template/client/client_template.c template/readme.txt README.md
	mkdir cnetlib-v$(VERSION)
	cp -r include lib template README.md LICENSE cnetlib-v$(VERSION)/
	zip -r cnetlib-v$(VERSION).zip cnetlib-v$(VERSION)/
	rm -rf cnetlib-v$(VERSION)/



#delete build artifacts
clean:
	rm -f src/client/csnl.o src/server/ssnl.o
