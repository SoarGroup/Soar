/*******************************************************************************
 * Author : Sreenivasa R. Viswanadha <sreeni@metamata.com>
 * Modified version: there was a bug in TypeTable.
 * Modificatio made by: Ferenc Magyar <magyar@inf.u-szeged.hu>
 *			MTA-JATE Research Group on Artificial Intelligence
 *			11/20/97
 *
 * Last change:  M    19 Oct 97    4:27 pm
*******************************************************************************/

import java.util.Hashtable;

public class Scope
{
   /**
    * Name of the scope (set only for class/function scopes).
	Last change:  M    19 Oct 97    4:21 pm
    */
   public String scopeName;

   /**
    * Indicates whether this is a class scope or not.
    */
   boolean type;     // Indicates if this is a type.

   /**
    * (partial) table of type symbols introduced in this scope.
    */
   Hashtable typeTable = new Hashtable();

   /**
    * Parent scope. (null if it is the global scope).
    */
   Scope parent;

   /**
    * Creates a scope object with a given name.
    */
   public Scope(String name, boolean isType, Scope p)
   {
      scopeName = name;
      type = isType;
      parent = p;
   }

   /**
    * Creates an unnamed scope (like for compound statements).
    */
   public Scope(Scope p)
   {
      type = false;
      parent = p;
   }

   /**
    * Inserts a name into the table to say that it is the name of a type.
    * There was a bug. The simple type declarartion override the existing type
    * definition.
    * Correction made: 11/20/97 by Ferenc Magyar <magyar@inf.u-szeged.hu>
    */
   public void PutTypeName(String name)
   {
     if (typeTable.get(name)==null )
        typeTable.put(name, name);
   }

   /**
    * A type with a scope (class/struct/union).
    */
   public void PutTypeName(String name, Scope sc)
   {
      typeTable.put(name, sc);
   }

   /** 
    * Checks if a given name is the name of a type in this scope.
    */
   public boolean IsTypeName(String name)
   {
      return typeTable.get(name) != null;
   }

   public Scope GetScope(String name)
   {
      Object sc = typeTable.get(name);

      if (sc instanceof Scope || sc instanceof ClassScope)
         return (Scope)sc;

      return null;
   }
}
