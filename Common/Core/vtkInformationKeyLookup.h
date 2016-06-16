/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformationKeyLookup.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkInformationKeyLookup - Find vtkInformationKeys from name and
// location strings.

#ifndef vtkInformationKeyLookup_h
#define vtkInformationKeyLookup_h

#include "vtkCommonCoreModule.h" // For export macro
#include "vtkObject.h"

#include <map> // For std::map
#include <utility> // For std::pair
#include <string> // For std::string

class vtkInformationKey;

class VTKCOMMONCORE_EXPORT vtkInformationKeyLookup: public vtkObject
{
public:
    static vtkInformationKeyLookup* New();
    vtkTypeMacro(vtkInformationKeyLookup, vtkObject)

    // Description:
    // Lists all known keys.
    virtual void PrintSelf(ostream &os, vtkIndent indent);

    // Description:
    // Find an information key from name and location strings. For example,
    // Find("GUI_HIDE", "vtkAbstractArray") returns vtkAbstractArray::GUI_HIDE.
    // Note that this class only knows about keys in modules that are currently
    // linked to the running executable.
    static vtkInformationKey* Find(const std::string &name,
                                   const std::string &location);

protected:
    vtkInformationKeyLookup();
    ~vtkInformationKeyLookup();

    friend class vtkInformationKey;

    // Description:
    // Add a key to the KeyMap. This is done automatically in the
    // vtkInformationKey constructor.
    static void RegisterKey(vtkInformationKey *key,
                            const std::string &name,
                            const std::string &location);

private:
    vtkInformationKeyLookup(const vtkInformationKeyLookup&); // Not implemented
    void operator=(const vtkInformationKeyLookup&); // Not implemented

    typedef std::pair<std::string, std::string> Identifier; // Location, Name
    typedef std::map<Identifier, vtkInformationKey*> KeyMap;

    // Using a static function / variable here to ensure static initialization
    // works as intended, since key objects are static, too.
    static KeyMap& Keys();
};

#endif // vtkInformationKeyLookup_h
