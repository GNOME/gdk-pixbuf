#!/usr/bin/python
#
# Utility script to generate .pc files for GDK-Pixbuf
# for Visual Studio builds, to be used for
# building introspection files

# Author: Fan, Chun-wei
# Date: April 22, 2016

import os
import sys
import argparse

from replace import replace_multi, replace
from pc_base import BasePCItems

def main(argv):
    base_pc = BasePCItems()
    base_pc.setup(argv)
    pkg_replace_items = {'@GDK_PIXBUF_API_VERSION@': '2.0',
                         '@GDK_PIXBUF_BINARY_VERSION@': '2.10.0',
                         '@PNG_DEP_CFLAGS_PACKAGES@': '',
                         '@GDK_PIXBUF_EXTRA_LIBS@': '',
                         '@GDK_PIXBUF_EXTRA_CFLAGS@': ''}

    pkg_replace_items.update(base_pc.base_replace_items)

    # Generate gdk-pixbuf-2.0.pc.tmp to replace the module directory
    replace_multi(base_pc.top_srcdir + '/gdk-pixbuf-2.0.pc.in',
                  base_pc.srcdir + '/gdk-pixbuf-2.0.pc.tmp',
                  pkg_replace_items)
    replace(base_pc.srcdir + '/gdk-pixbuf-2.0.pc.tmp',
            base_pc.srcdir + '/gdk-pixbuf-2.0.pc',
            '${prefix}/lib/gdk-pixbuf',
            '${exec_prefix}/lib/gdk-pixbuf')
    os.unlink(base_pc.srcdir + '/gdk-pixbuf-2.0.pc.tmp')

if __name__ == '__main__':
    sys.exit(main(sys.argv))
