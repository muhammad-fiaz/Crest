---
name: Documentation Issue
description: Report issues with documentation, examples, or guides
title: "[DOCS] "
labels: ["documentation", "help-wanted"]
assignees: []
body:
  - type: markdown
    attributes:
      value: |
        ## Documentation Issue
        
        Help us improve our documentation! Report unclear sections, missing information, or suggest improvements.

  - type: dropdown
    id: issue-type
    attributes:
      label: Documentation Issue Type
      description: What type of documentation issue are you reporting?
      options:
        - Unclear or confusing explanation
        - Missing information
        - Outdated information
        - Code example issue
        - Broken link
        - Typos or grammar errors
        - Other (please specify)
    validations:
      required: true

  - type: input
    id: page-url
    attributes:
      label: Page URL
      description: Which documentation page has the issue?
      placeholder: "https://muhammad-fiaz.github.io/crest/... or relative path"
    validations:
      required: true

  - type: textarea
    id: description
    attributes:
      label: Description
      description: Describe the documentation issue in detail.
      placeholder: "What's unclear? What's missing? What needs to be updated?"
    validations:
      required: true

  - type: textarea
    id: current-text
    attributes:
      label: Current Text/Content
      description: Quote the problematic text or describe the current content.
      placeholder: "Copy the text that's unclear or incorrect here."

  - type: textarea
    id: suggested-fix
    attributes:
      label: Suggested Fix
      description: How should this be improved?
      placeholder: "Provide your suggested improvement or correction."

  - type: dropdown
    id: urgency
    attributes:
      label: Urgency
      description: How urgent is this documentation fix?
      options:
        - Low - Minor improvement
        - Medium - Affects usability
        - High - Blocks understanding or usage
        - Critical - Misleading or dangerous information
    validations:
      required: true

  - type: textarea
    id: additional-context
    attributes:
      label: Additional Context
      description: Any additional context or information.
      placeholder: "Screenshots, related issues, etc."

  - type: checkboxes
    id: checklist
    attributes:
      label: Checklist
      description: Please check the following before submitting
      options:
        - label: I have checked that this documentation issue hasn't been reported before
          required: true
        - label: I am willing to help fix this documentation issue
          required: false