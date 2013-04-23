import subprocess


# fields values
pkgname = 'fspc'
pkgver = '1.0'
url = 'http://127.0.0.1/'
targz = pkgname + '-' + pkgver + '.tar.gz'

# output of the PKGBUILD
f = open("PKGBUILD", "w")

f.write('# Maintainer: Vincenzo Maffione <v.maffione@gmail.com>\n');
f.write('pkgname=%s\n' % (pkgname, ));
f.write('pkgver=%s\n' % (pkgver, ));
f.write('pkgrel=1\n');
f.write('epoch=\n');
f.write('pkgdesc="An FSP compiler and LTS analysis tool"\n');
f.write('arch=(\'i686\' \'x86_64\')\n');
f.write('url="%s"\n' % (url, ));
f.write('license=(\'GPL\')\n');
f.write('groups=()\n');
f.write('depends=()\n');
f.write('makedepends=()\n');
f.write('checkdepends=()\n');
f.write('optdepends=()\n');
f.write('provides=()\n');
f.write('conflicts=()\n');
f.write('replaces=()\n');
f.write('backup=()\n');
f.write('options=()\n');
f.write('install=\n');
f.write('changelog=\n');
f.write('source=("%s$pkgname-$pkgver.tar.gz")\n' % (url, ));
f.write('noextract=()\n');

#compute md5 checksum
ret = subprocess.check_output(["md5sum", targz])
f.write('md5sums=(\'%s\')\n' % ( ret[0:32].decode('latin-1'), ))


f.write('\nbuild() {\n\tcd "$srcdir"\n\tmake || return 1 \n}\n');

f.write('\ncheck() {\n\techo "nothing to check"\n}\n');

f.write('\npackage() {\n\tcd "$srcdir"\n\tmkdir -p "$pkgdir/usr/bin"\n\tcp fspc "$pkgdir/usr/bin"\n}\n');

f.write('# vim:set ts=2 sw=2 et:\n')
f.close()
