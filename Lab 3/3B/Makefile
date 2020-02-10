# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

EXEC=lab3b
SRC=lab3b.py

TAR=lab3b-123456789.tar.gz

build: $(EXEC)

$(EXEC):
	rm -f $(EXEC)
	ln -s $(SRC) $(EXEC)
	chmod +x $(EXEC)
	echo "lab3b link successfully made"

dist:
	tar -cvzf $(TAR) $(SRC) Makefile README

clean:
	rm -f $(EXEC) $(TAR)
