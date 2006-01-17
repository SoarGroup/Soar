#'$Id$'

# COPYRIGHT (C) 1998  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and
# that both that copyright notice and this permission notice appear in
# supporting documentation.
# THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
# ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

try: #Following switcheroo ensures that if PIL is installed it's used instead
    # of the HTMLgen-bundled copies.
    import Image
except ImportError:
    import ImageH
    Image = ImageH
    del ImageH

def imgsize(path):
    """Returns the (width, height) in pixels of the given image file.
    Understands GIF, JPEG, and PNG formats.
    """
    pict = Image.open(path)  #Too easy.  Thanks Fredrik!
    width, height = pict.size
    return width, height

