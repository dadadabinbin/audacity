/**********************************************************************

  Audacity: A Digital Audio Editor

  CommandManager.h

  Brian Gunlogson
  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_COMMAND_MANAGER__
#define __AUDACITY_COMMAND_MANAGER__

#include "../Experimental.h"

#include "CommandFunctors.h"
#include "CommandFlag.h"

#include "../MemoryX.h"
#include <vector>
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/menu.h>
#include <wx/hashmap.h>

#include "../xml/XMLTagHandler.h"

#include "audacity/Types.h"

#ifndef __AUDACITY_OLD_STD__
#include <unordered_map>
#endif

struct MenuBarListEntry
{
   MenuBarListEntry(const wxString &name_, wxMenuBar *menubar_)
      : name(name_), menubar(menubar_)
   {}

   wxString name;
   wxMenuBar *menubar; // This structure does not assume memory ownership!
};

struct SubMenuListEntry
{
   SubMenuListEntry(const wxString &name_, std::unique_ptr<wxMenu> &&menu_)
      : name(name_), menu( std::move(menu_) )
   {}

   SubMenuListEntry(SubMenuListEntry &&that)
      : name(std::move(that.name))
      , menu(std::move(that.menu))
   {
   }

   wxString name;
   std::unique_ptr<wxMenu> menu;
};

struct CommandListEntry
{
   int id;
   wxString name;
   wxString key;
   wxString defaultKey;
   wxString label;
   wxString labelPrefix;
   wxString labelTop;
   wxMenu *menu;
   CommandHandlerFinder finder;
   CommandFunctorPointer callback;
   CommandParameter parameter;
   bool multi;
   int index;
   int count;
   bool enabled;
   bool skipKeydown;
   bool wantKeyup;
   bool isGlobal;
   bool isOccult;
   CommandFlag flags;
   CommandMask mask;
};

using MenuBarList = std::vector < MenuBarListEntry >;

// to do: remove the extra indirection when Mac compiler moves to newer version
using SubMenuList = std::vector < movable_ptr<SubMenuListEntry> >;

// This is an array of pointers, not structures, because the hash maps also point to them,
// so we don't want the structures to relocate with vector operations.
using CommandList = std::vector<movable_ptr<CommandListEntry>>;

using CommandNameHash = std::unordered_map<wxString, CommandListEntry*>;
using CommandIDHash = std::unordered_map<int, CommandListEntry*>;

class AudacityProject;

class AUDACITY_DLL_API CommandManager final : public XMLTagHandler
{
 public:

   //
   // Constructor / Destructor
   //

   CommandManager();
   virtual ~CommandManager();

   CommandManager(const CommandManager&) PROHIBITED;
   CommandManager &operator= (const CommandManager&) PROHIBITED;

   void SetMaxList();
   void PurgeData();

   //
   // Creating menus and adding commands
   //

   std::unique_ptr<wxMenuBar> AddMenuBar(const wxString & sMenu);

   // You may either called SetCurrentMenu later followed by ClearCurrentMenu,
   // or else BeginMenu followed by EndMenu.  Don't mix them.
   void BeginMenu(const wxString & tName);
   void EndMenu();

   wxMenu* BeginSubMenu(const wxString & tName);
   void EndSubMenu();

   void InsertItem(const wxString & name,
                   const wxString & label,
                   CommandHandlerFinder finder,
                   CommandFunctorPointer callback,
                   const wxString & after,
                   int checkmark = -1);

   void AddItemList(const wxString & name,
                    const wxArrayString & labels,
                    CommandHandlerFinder finder,
                    CommandFunctorPointer callback);

   void AddCheck(const wxChar *name,
                 const wxChar *label,
                 CommandHandlerFinder finder,
                 CommandFunctorPointer callback,
                 int checkmark = 0);

   void AddCheck(const wxChar *name,
                 const wxChar *label,
                 CommandHandlerFinder finder,
                 CommandFunctorPointer callback,
                 int checkmark,
                 CommandFlag flags,
                 CommandMask mask);

   void AddItem(const wxChar *name,
                const wxChar *label,
                CommandHandlerFinder finder,
                CommandFunctorPointer callback,
                CommandFlag flags = NoFlagsSpecifed,
                CommandMask mask   = NoFlagsSpecifed,
                const CommandParameter &parameter = CommandParameter{});

   void AddItem(const wxChar *name,
                const wxChar *label_in,
                CommandHandlerFinder finder,
                CommandFunctorPointer callback,
                const wxChar *accel,
                CommandFlag flags = NoFlagsSpecifed,
                CommandMask mask   = NoFlagsSpecifed,
                int checkmark = -1,
                const CommandParameter &parameter = CommandParameter{});

   void AddSeparator();

   // A command doesn't actually appear in a menu but might have a
   // keyboard shortcut.
   void AddCommand(const wxChar *name,
                   const wxChar *label,
                   CommandHandlerFinder finder,
                   CommandFunctorPointer callback,
                   CommandFlag flags = NoFlagsSpecifed,
                   CommandMask mask   = NoFlagsSpecifed);

   void AddCommand(const wxChar *name,
                   const wxChar *label,
                   CommandHandlerFinder finder,
                   CommandFunctorPointer callback,
                   const wxChar *accel,
                   CommandFlag flags = NoFlagsSpecifed,
                   CommandMask mask   = NoFlagsSpecifed);

   void AddGlobalCommand(const wxChar *name,
                         const wxChar *label,
                         CommandHandlerFinder finder,
                         CommandFunctorPointer callback,
                         const wxChar *accel);
   //
   // Command masks
   //

   // For NEW items/commands
   void SetDefaultFlags(CommandFlag flags, CommandMask mask);
   CommandFlag GetDefaultFlags() const { return mDefaultFlags; }
   CommandMask GetDefaultMask() const { return mDefaultMask; }

   void SetOccultCommands( bool bOccult);


   void SetCommandFlags(const wxString &name, CommandFlag flags, CommandMask mask);
   void SetCommandFlags(const wxChar **names,
                        CommandFlag flags, CommandMask mask);
   // Pass multiple command names as const wxChar *, terminated by NULL
   void SetCommandFlags(CommandFlag flags, CommandMask mask, ...);

   //
   // Modifying menus
   //

   void EnableUsingFlags(CommandFlag flags, CommandMask mask);
   void Enable(const wxString &name, bool enabled);
   void Check(const wxString &name, bool checked);
   void Modify(const wxString &name, const wxString &newLabel);

   // You may either called SetCurrentMenu later followed by ClearCurrentMenu,
   // or else BeginMenu followed by EndMenu.  Don't mix them.
   void SetCurrentMenu(wxMenu * menu);
   void ClearCurrentMenu();

   //
   // Modifying accelerators
   //

   void SetKeyFromName(const wxString &name, const wxString &key);
   void SetKeyFromIndex(int i, const wxString &key);

   //
   // Executing commands
   //

   // "permit" allows filtering even if the active window isn't a child of the project.
   // Lyrics and MixerTrackCluster classes use it.
   bool FilterKeyEvent(AudacityProject *project, const wxKeyEvent & evt, bool permit = false);
   bool HandleMenuID(int id, CommandFlag flags, CommandMask mask);
   bool HandleTextualCommand(const wxString & Str, CommandFlag flags, CommandMask mask);

   //
   // Accessing
   //

   void GetCategories(wxArrayString &cats);
   void GetAllCommandNames(wxArrayString &names, bool includeMultis);
   void GetAllCommandLabels(wxArrayString &labels, bool includeMultis);
   void GetAllCommandData(
      wxArrayString &names, wxArrayString &keys, wxArrayString &default_keys,
      wxArrayString &labels, wxArrayString &categories,
#if defined(EXPERIMENTAL_KEY_VIEW)
      wxArrayString &prefixes,
#endif
      bool includeMultis);

   wxString GetLabelFromName(const wxString &name);
   wxString GetPrefixedLabelFromName(const wxString &name);
   wxString GetCategoryFromName(const wxString &name);
   wxString GetKeyFromName(const wxString &name) const;
   wxString GetDefaultKeyFromName(const wxString &name);

   bool GetEnabled(const wxString &name);

#if defined(__WXDEBUG__)
   void CheckDups();
#endif

   //
   // Loading/Saving
   //

   void WriteXML(XMLWriter &xmlFile) const /* not override */;
   void TellUserWhyDisallowed(const wxString & Name, CommandFlag flagsGot, CommandFlag flagsRequired);

   ///
   /// Formatting summaries that include shortcut keys
   ///
   using LocalizedCommandName = std::pair<wxString, const wxChar*>;
   using LocalizedCommandNameVector = std::vector<LocalizedCommandName>;
   wxString DescribeCommandsAndShortcuts
      (// An array of paired user-visible strings, and
       // non-user-visible command names.  If a shortcut key is defined
       // for the command, then it is appended, parenthesized, after the
       // user-visible string.
       const LocalizedCommandNameVector &commands) const;

protected:

   //
   // Creating menus and adding commands
   //

   int NextIdentifier(int ID);
   CommandListEntry *NewIdentifier(const wxString & name,
                                   const wxString & label,
                                   wxMenu *menu,
                                   CommandHandlerFinder finder,
                                   CommandFunctorPointer callback,
                                   bool multi,
                                   int index,
                                   int count);
   CommandListEntry *NewIdentifier(const wxString & name,
                                   const wxString & label,
                                   const wxString & accel,
                                   wxMenu *menu,
                                   CommandHandlerFinder finder,
                                   CommandFunctorPointer callback,
                                   bool multi,
                                   int index,
                                   int count,
                                   const CommandParameter &parameter);

   //
   // Executing commands
   //

   bool HandleCommandEntry(const CommandListEntry * entry, CommandFlag flags, CommandMask mask, const wxEvent * evt = NULL);

   //
   // Modifying
   //

   void Enable(CommandListEntry *entry, bool enabled);

   //
   // Accessing
   //

   wxMenuBar * CurrentMenuBar() const;
   wxMenuBar * GetMenuBar(const wxString & sMenu) const;
   wxMenu * CurrentSubMenu() const;
   wxMenu * CurrentMenu() const;
   wxString GetLabel(const CommandListEntry *entry) const;
   wxString GetLabelWithDisabledAccel(const CommandListEntry *entry) const;

   //
   // Loading/Saving
   //

   bool HandleXMLTag(const wxChar *tag, const wxChar **attrs) override;
   void HandleXMLEndTag(const wxChar *tag) override;
   XMLTagHandler *HandleXMLChild(const wxChar *tag) override;

private:
   // mMaxList only holds shortcuts that should not be added (by default).
   wxSortedArrayString mMaxListOnly;

   MenuBarList  mMenuBarList;
   SubMenuList  mSubMenuList;
   CommandList  mCommandList;
   CommandNameHash  mCommandNameHash;
   CommandNameHash  mCommandKeyHash;
   CommandIDHash  mCommandIDHash;
   int mCurrentID;
   int mXMLKeysRead;

   bool mbSeparatorAllowed; // false at the start of a menu and immediately after a separator.

   wxString mCurrentMenuName;
   std::unique_ptr<wxMenu> uCurrentMenu;
   wxMenu *mCurrentMenu {};

   CommandFlag mDefaultFlags;
   CommandMask mDefaultMask;
   bool bMakingOccultCommands;
};

#endif
