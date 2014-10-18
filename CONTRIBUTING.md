# How to contribute

Third-party patches/pull requests are great for Soar.  Soar is a very
complicated Artificial Intelligence Framework and not everything we
want to do gets done.  However, by using pull requests, people can
help contribute to Soar and improve it in ways that we cannot.  We
want to make it easy to contribute to Soar, even though there are
parts, such as the Kernel, which are very hard to contribute to.  In
order to make this possible, there are a few guidelines that we need
contributors to follow so that we can have a chance of keeping on top
of things.

## Getting Started
## 
* Make sure you have a [GitHub account](https://github.com/signup/free)
* Read [this](http://nvie.com/posts/a-successful-git-branching-model/) and
[this](https://www.atlassian.com/git/workflows#!workflow-gitflow) for
  information on how our git repository is structured.


### Making Big Kernel Changes

The Soar Kernel is a very ancient (in computing terms) and optimized
piece of code.  If you are making changes to it, make sure that you are
careful to not break anything.  Also, if you have some doubt whether you
know what you are doing while modifying the kernel, talk with someone in
Soar Group who will happily help you out.

If you are making big changes, talk with someone in the Soar Group
organization to ensure that what you are doing is worthwhile, assuming
you are planning on trying to get it back into the main repository.

### Fixing an issue

* Make sure there isn't already a ticket for the issue
* If there isn't, submit one before doing any work on fixing it.
* Clearly describe the issue including steps to reproduce when
  it is a bug.  If you have any code which demonstrates the issue,
      please include that as an attachment.
* Note the earliest version that you know has the issue
    + Ideally reference the commit which created the issue.
* Fork the repository on GitHub

## Making Changes

* Create a branch from where you want to base your work.
    - This is usually branched off of the development branch.
    - If you are not using the development branch, ask someone in SoarGroup to
make sure it's what you want.
* To quickly create a branch based on development:

`git checkout -b origin/my_contribution development`

* Make commits of [logically separable
changesets](http://git-scm.com/book/en/Distributed-Git-Contributing-to-a-Project
).
    - "Don't code for a whole weekend on five different issues and then submit
them all as one massive commit on Monday."
* Check for unnecessary whitespace with `git diff --check` before committing.
* Make sure your commit messages are in the proper format.

````
Short (50 chars or less) summary of changes (including Fixed #<Issue
Number> if fixing an issue)

More detailed explanatory text, if necessary.  Wrap it to about 72
characters or so.  The first line is treated as the subject of an email
and the rest of the text as the body.  The blank line separating the
summary from the body is critical (unless you omit the body entirely);
tools like rebase can get confused if you run the two together.

Further paragraphs come after blank lines.

- Bullet points are okay, too
- After all this is actually GitHub markdown parsed
````

* Make sure you follow the coding style:
    - We, in general, try to use [Allman
Style](https://en.wikipedia.org/wiki/Indent_style#Allman_style) with the
following modifications
       - 4 spaces per indent.  
       - No tabs.
       - Use linux line endings.
       - Brackets around one-line blocks.
       - Indent public/private/protected
       - Indent cases in switch
       - Indent within namespace scope specifier
       - Pointer and reference operators attached to type (e.g. (Symbol* sym))
       - Spaces between operators (e.g. "x + y")
       - Space after control functions (e.g "for (i = 0; i == 1; ++i)")
       - No space before/after function parameters (e.g. void myFun(Symbol* sym))
    - We tend to use underlines ( _ ) in our names.
    - All files are lower cased.

** In the Soar folder, there is a mac/linux script called format_soar.sh that
will reformat only the appropriate source files in SoarSuite.  It can also be
used to format individual files.  You must have astyle installed for it to work.
 (http://astyle.sourceforge.net/)
    
**Do not change file names without reason, especially lower case to upper
case**.

* Make sure you have added the code demonstrating the issue as a test to
  UnitTests.  This can be found in Tests/.

* Make sure your items are namespaced if you're making new or big changes.
    - Typically soar::[Name, ie. lexer or semantic_memory]

* Run _all_ the tests to assure nothing was accidentally broken. If you
  break the project in your git pull request, Jenkins [(our build
system)](http://soar-jenkins.eecs.umich.edu:8080)
  will let you know.  Until Jenkins passes your pull request, it **WILL NOT**
  be merged in.

## Submitting Changes

* Push your changes to a branch in your fork of the repository.
* Submit a pull request to the Soar repository in the SoarGroup
organization.
    - Please describe what your pull request does in clear, concise,
    and precise english.
* If your pull request builds in Jenkins, then we will consider merging
  it in.  Don't worry if we take a little while to get around to it.  We
  look at all pull requests but some may take longer than others to look at.

# Additional Resources

* [General GitHub documentation](http://help.github.com/)
* [GitHub pull request
documentation](http://help.github.com/send-pull-requests/)
* [How to GitHub: Fork, Branch, Track, Squash and Pull
Request](https://gun.io/blog/how-to-github-fork-branch-and-pull-request/)
* [Soar Home Page](http://soar.eecs.umich.edu/)
* [Soar Github Page](https://github.com/SoarGroup/Soar)

## References

* [Distributed Git - Contributing to a
Project](http://git-scm.com/book/en/Distributed-Git-Contributing-to-a-Project)
* [GitHub Contributing
Guidelines](https://github.com/blog/1184-contributing-guidelines)
* [Puppet
Contributing.md](https://github.com/puppetlabs/puppet/blob/master/CONTRIBUTING.
md)
