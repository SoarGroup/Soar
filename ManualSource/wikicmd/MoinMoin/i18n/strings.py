# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - lists of translateable strings

    MoinMoin uses some translateable strings that do not appear at other
    places in the source code (and thus, are not found by gettext when
    extracting translateable strings).
    Also, some strings need to be organized somehow.

    TODO i18n.strings / general:
    * fix other translations (can be done using ##master-page, but help
      from a native speaker would be the preferred solution)
    * delete other SystemPagesInXXXGroup if their po file is complete

    @copyright: 2009 MoinMoin:ThomasWaldmann
    @license: GNU GPL, see COPYING for details.
"""

_ = lambda x: x # dummy translation function

# Some basic pages used for every language, but we only need them once in English (don't translate!):
not_translated_system_pages = [
    'LanguageSetup',
    'InterWikiMap',
    'BadContent',
    'LocalBadContent',
    'EditedSystemPages',
    'LocalSpellingWords',
    'SystemAdmin',
    'SystemInfo',
    'ProjectTemplate',
    'ProjectGroupsTemplate',
    'PermissionDeniedPage',
]

essential_system_pages = [
    _('RecentChanges'),
    _('WikiTipOfTheDay'), # used by RecentChanges
    _('TitleIndex'),
    _('WordIndex'),
    _('FindPage'),
    _('MissingPage'),
    _('MissingHomePage'),
    _('WikiHomePage'), # used by CategoryHomepage

    # these are still in use, but should be killed:
    _('WikiName'), # linked from misc. help/tips pages
    _('WikiWikiWeb'), # used by FrontPage/WikiHomePage
]

optional_system_pages = [
    _('FrontPage'),
    _('WikiSandBox'),
    _('InterWiki'),
    _('AbandonedPages'),
    _('OrphanedPages'),
    _('WantedPages'),
    _('EventStats'),
    _('EventStats/HitCounts'),
    _('EventStats/Languages'),
    _('EventStats/UserAgents'),
    _('PageSize'),
    _('PageHits'),
    _('RandomPage'),
    _('XsltVersion'),
    _('FortuneCookies'), # use by RandomQuote macro
    _('WikiLicense'), # does not exist, but can be created by wiki admin
]

translated_system_pages = essential_system_pages + optional_system_pages

all_system_pages = not_translated_system_pages + translated_system_pages

essential_category_pages = [
    _('CategoryCategory'),
    _('CategoryHomepage'),
]

optional_category_pages = [
]

all_category_pages = essential_category_pages + optional_category_pages

essential_template_pages = [
    _('CategoryTemplate'),
    _('HomepageTemplate'),
]

optional_template_pages = [
    _('HelpTemplate'),
    _('HomepageReadWritePageTemplate'),
    _('HomepageReadPageTemplate'),
    _('HomepagePrivatePageTemplate'),
    _('HomepageGroupsTemplate'),
    _('SlideShowHandOutTemplate'),
    _('SlideShowTemplate'),
    _('SlideTemplate'),
    _('SyncJobTemplate'),
]

all_template_pages = essential_template_pages + optional_template_pages

# Installation / Configuration / Administration Help:
admin_pages = [
    _('HelpOnConfiguration'),
    _('HelpOnConfiguration/EmailSupport'),
    _('HelpOnConfiguration/SecurityPolicy'),
    _('HelpOnConfiguration/FileAttachments'),
    _('HelpOnConfiguration/SupplementationPage'),
    _('HelpOnConfiguration/SurgeProtection'),
    _('HelpOnConfiguration/UserPreferences'),
    _('HelpOnPackageInstaller'),
    _('HelpOnUpdatingPython'),
    _('HelpOnAdministration'),
    _('HelpOnAuthentication'),
    _('HelpOnAuthentication/ExternalCookie'),
    _('HelpOnMoinCommand'),
    _('HelpOnMoinCommand/ExportDump'),
    _('HelpOnNotification'),
    _('HelpOnSessions'),
    _('HelpOnUserHandling'),
    _('HelpOnXapian'),
]

# Stuff that should live on moinmo.in wiki:
obsolete_pages = [
]

essential_help_pages = [
    _('HelpOnMoinWikiSyntax'), # used by edit action
    _('HelpOnCreoleSyntax'), # used by edit action
    # HelpOnParsers/ReStructuredText/RstPrimer could be renamed and used in a similar way
]

optional_help_pages = [
    _('HelpOnFormatting'), # still needed?
    _('MoinMoin'),
    _('HelpContents'),
    _('HelpForBeginners'),
    _('HelpForUsers'),
    _('HelpIndex'),
    _('HelpOnAccessControlLists'),
    _('HelpOnActions'),
    _('HelpOnActions/AttachFile'),
    _('HelpOnAdmonitions'),
    _('HelpOnAutoAdmin'),
    _('HelpOnCategories'),
    _('HelpOnDictionaries'),
    _('HelpOnDrawings'),
    _('HelpOnEditLocks'),
    _('HelpOnEditing'), # used by edit action!
    _('HelpOnEditing/SubPages'),
    _('HelpOnGraphicalEditor'),
    _('HelpOnGroups'),
    _('HelpOnHeadlines'),
    _('HelpOnImages'),
    _('HelpOnLanguages'),
    _('HelpOnLinking'),
    _('HelpOnLinking/NotesLinks'),
    _('HelpOnLists'),
    _('HelpOnLogin'),
    _('HelpOnMacros'),
    _('HelpOnMacros/EmbedObject'),
    _('HelpOnMacros/Include'),
    _('HelpOnMacros/MailTo'),
    _('HelpOnMacros/MonthCalendar'),
    _('HelpOnNavigation'),
    _('HelpOnOpenIDProvider'),
    _('HelpOnPageCreation'),
    _('HelpOnPageDeletion'),
    _('HelpOnParsers'),
    _('HelpOnParsers/ReStructuredText'),
    _('HelpOnParsers/ReStructuredText/RstPrimer'),
    _('HelpOnProcessingInstructions'),
    _('HelpOnRules'),
    _('HelpOnSearching'),
    _('HelpOnSlideShows'),
    _('HelpOnSlideShows/000 Introduction'),
    _('HelpOnSlideShows/100 Creating the slides'),
    _('HelpOnSlideShows/900 Last but not least: Running your presentation'),
    _('HelpOnSmileys'),
    _('HelpOnSpam'),
    _('HelpOnSpellCheck'),
    _('HelpOnSuperUser'),
    _('HelpOnSynchronisation'),
    _('HelpOnTables'),
    _('HelpOnTemplates'),
    _('HelpOnThemes'),
    _('HelpOnUserPreferences'),
    _('HelpOnVariables'),
    _('HelpOnXmlPages'),
    _('HelpOnComments'),
    _('HelpOnSubscribing'),

    # these are still in use, but should be killed:
    _('CamelCase'), # linked from misc. help/course pages
]

all_help_pages = essential_help_pages + optional_help_pages

# Wiki Course:
course_pages = [
    _('WikiCourse'),
    _('WikiCourse/01 What is a MoinMoin wiki?'),
    _('WikiCourse/02 Finding information'),
    _('WikiCourse/03 Staying up to date'),
    _('WikiCourse/04 Creating a wiki account'),
    _('WikiCourse/05 User preferences'),
    _('WikiCourse/06 Your own wiki homepage'),
    _('WikiCourse/07 The text editor'),
    _('WikiCourse/08 Hot Keys'),
    _('WikiCourse/10 Text layout with wiki markup'),
    _('WikiCourse/11 Paragraphs'),
    _('WikiCourse/12 Headlines'),
    _('WikiCourse/13 Lists'),
    _('WikiCourse/14 Text styles'),
    _('WikiCourse/15 Tables'),
    _('WikiCourse/16 Wiki internal links'),
    _('WikiCourse/17 External links'),
    _('WikiCourse/18 Attachments'),
    _('WikiCourse/19 Symbols'),
    _('WikiCourse/20 Dynamic content'),
    _('WikiCourse/21 Macros'),
    _('WikiCourse/22 Parsers'),
    _('WikiCourse/23 Actions'),
    _('WikiCourse/30 The graphical editor'),
    _('WikiCourse/40 Creating more pages'),
    _('WikiCourse/50 Wiki etiquette'),
    _('WikiCourse/51 Applications'),
    _('WikiCourse/52 Structure in the wiki'),
    _('WikiCourseHandOut'),
]

essential_pages = (
    essential_system_pages +
    essential_category_pages +
    essential_template_pages +
    essential_help_pages
)

optional_pages = (
    optional_system_pages +
    optional_category_pages +
    optional_template_pages +
    optional_help_pages
)

all_pages = (
    all_system_pages +
    all_category_pages +
    all_template_pages +
    all_help_pages +
    admin_pages +
    obsolete_pages +
    course_pages
)

# an list of page sets translators should look at,
# ordered in the order translators should look at them
pagesets = [
    'not_translated_system_pages',
    'essential_system_pages',
    'essential_help_pages',
    'essential_category_pages',
    'essential_template_pages',
    'essential_pages',
    'optional_system_pages',
    'optional_help_pages',
    'optional_category_pages',
    'optional_template_pages',
    'optional_pages',
    'translated_system_pages',
    'all_system_pages',
    'all_help_pages',
    'all_category_pages',
    'all_template_pages',
    'admin_pages',
    'course_pages',
    'obsolete_pages',
    'all_pages',
]

# we use Sun at index 0 and 7 to be compatible with EU and US day indexing
# schemes, like it is also done within crontab entries:
weekdays = [_('Sun'), _('Mon'), _('Tue'), _('Wed'), _('Thu'), _('Fri'), _('Sat'), _('Sun')]

actions = [
    _('AttachFile'),
    _('DeletePage'),
    _('LikePages'),
    _('LocalSiteMap'),
    _('RenamePage'),
    _('SpellCheck'),
]

misc = [
    # the editbar link text of the default supplementation page link:
    _('Discussion'),
]

del _ # delete the dummy translation function

