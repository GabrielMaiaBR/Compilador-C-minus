sudo ./killbuild
cd build
sudo bash ../compLabAluno.bash
sudo chmod -R ugo+rw *
sudo chmod -R ugo+rw ../alunoout
make
make lexdiff
make ddiff

# meld ../alunodetail/ ../detail/ &
