﻿// Copyright (c) Microsoft Corporation. All rights reserved. See License.txt in the project root for license information.

using System.Collections.Generic;
using System.Web.TestUtil;
using Xunit;
using Assert = Microsoft.TestCommon.AssertEx;

namespace System.Web.Mvc.Ajax.Test
{
    public class AjaxOptionsTest
    {
        [Fact]
        public void InsertionModeProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestEnumProperty(options, "InsertionMode", InsertionMode.Replace, false);
        }

        [Fact]
        public void InsertionModePropertyExceptionText()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            Assert.ThrowsArgumentOutOfRange(
                delegate { options.InsertionMode = (InsertionMode)4; },
                "value",
                @"Specified argument was out of the range of valid values.");
        }

        [Fact]
        public void InsertionModeStringTests()
        {
            // Act & Assert
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.Replace }.InsertionModeString, "Sys.Mvc.InsertionMode.replace");
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.InsertAfter }.InsertionModeString, "Sys.Mvc.InsertionMode.insertAfter");
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.InsertBefore }.InsertionModeString, "Sys.Mvc.InsertionMode.insertBefore");
        }

        [Fact]
        public void InsertionModeUnobtrusiveTests()
        {
            // Act & Assert
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.Replace }.InsertionModeUnobtrusive, "replace");
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.InsertAfter }.InsertionModeUnobtrusive, "after");
            Assert.Equal(new AjaxOptions { InsertionMode = InsertionMode.InsertBefore }.InsertionModeUnobtrusive, "before");
        }

        [Fact]
        public void HttpMethodProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestStringProperty(options, "HttpMethod", String.Empty);
        }

        [Fact]
        public void OnBeginProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestStringProperty(options, "OnBegin", String.Empty);
        }

        [Fact]
        public void OnFailureProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestStringProperty(options, "OnFailure", String.Empty);
        }

        [Fact]
        public void OnSuccessProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestStringProperty(options, "OnSuccess", String.Empty);
        }

        [Fact]
        public void ToJavascriptStringWithEmptyOptions()
        {
            string s = (new AjaxOptions()).ToJavascriptString();
            Assert.Equal("{ insertionMode: Sys.Mvc.InsertionMode.replace }", s);
        }

        [Fact]
        public void ToJavascriptString()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions
            {
                InsertionMode = InsertionMode.InsertBefore,
                Confirm = "confirm",
                HttpMethod = "POST",
                LoadingElementId = "loadingElement",
                UpdateTargetId = "someId",
                Url = "http://someurl.com",
                OnBegin = "some_begin_function",
                OnComplete = "some_complete_function",
                OnFailure = "some_failure_function",
                OnSuccess = "some_success_function",
            };

            // Act
            string s = options.ToJavascriptString();

            // Assert
            Assert.Equal("{ insertionMode: Sys.Mvc.InsertionMode.insertBefore, " +
                         "confirm: 'confirm', " +
                         "httpMethod: 'POST', " +
                         "loadingElementId: 'loadingElement', " +
                         "updateTargetId: 'someId', " +
                         "url: 'http://someurl.com', " +
                         "onBegin: Function.createDelegate(this, some_begin_function), " +
                         "onComplete: Function.createDelegate(this, some_complete_function), " +
                         "onFailure: Function.createDelegate(this, some_failure_function), " +
                         "onSuccess: Function.createDelegate(this, some_success_function) }", s);
        }

        [Fact]
        public void ToJavascriptStringEscapesQuotesCorrectly()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions
            {
                InsertionMode = InsertionMode.InsertBefore,
                Confirm = @"""confirm""",
                HttpMethod = "POST",
                LoadingElementId = "loading'Element'",
                UpdateTargetId = "someId",
                Url = "http://someurl.com",
                OnBegin = "some_begin_function",
                OnComplete = "some_complete_function",
                OnFailure = "some_failure_function",
                OnSuccess = "some_success_function",
            };

            // Act
            string s = options.ToJavascriptString();

            // Assert
            Assert.Equal("{ insertionMode: Sys.Mvc.InsertionMode.insertBefore, " +
                         @"confirm: '""confirm""', " +
                         "httpMethod: 'POST', " +
                         @"loadingElementId: 'loading\'Element\'', " +
                         "updateTargetId: 'someId', " +
                         "url: 'http://someurl.com', " +
                         "onBegin: Function.createDelegate(this, some_begin_function), " +
                         "onComplete: Function.createDelegate(this, some_complete_function), " +
                         "onFailure: Function.createDelegate(this, some_failure_function), " +
                         "onSuccess: Function.createDelegate(this, some_success_function) }", s);
        }

        [Fact]
        public void ToJavascriptStringWithOnlyUpdateTargetId()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions { UpdateTargetId = "someId" };

            // Act
            string s = options.ToJavascriptString();

            // Assert
            Assert.Equal("{ insertionMode: Sys.Mvc.InsertionMode.replace, updateTargetId: 'someId' }", s);
        }

        [Fact]
        public void ToJavascriptStringWithUpdateTargetIdAndExplicitInsertionMode()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions { InsertionMode = InsertionMode.InsertAfter, UpdateTargetId = "someId" };

            // Act
            string s = options.ToJavascriptString();

            // Assert
            Assert.Equal("{ insertionMode: Sys.Mvc.InsertionMode.insertAfter, updateTargetId: 'someId' }", s);
        }

        [Fact]
        public void ToUnobtrusiveHtmlAttributesWithEmptyOptions()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act
            IDictionary<string, object> attributes = options.ToUnobtrusiveHtmlAttributes();

            // Assert
            Assert.Single(attributes);
            Assert.Equal("true", attributes["data-ajax"]);
        }

        [Fact]
        public void ToUnobtrusiveHtmlAttributes()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions
            {
                InsertionMode = InsertionMode.InsertBefore,
                Confirm = "confirm",
                HttpMethod = "POST",
                LoadingElementId = "loadingElement",
                LoadingElementDuration = 450,
                UpdateTargetId = "someId",
                Url = "http://someurl.com",
                OnBegin = "some_begin_function",
                OnComplete = "some_complete_function",
                OnFailure = "some_failure_function",
                OnSuccess = "some_success_function",
            };

            // Act
            var attributes = options.ToUnobtrusiveHtmlAttributes();

            // Assert
            Assert.Equal(12, attributes.Count);
            Assert.Equal("true", attributes["data-ajax"]);
            Assert.Equal("confirm", attributes["data-ajax-confirm"]);
            Assert.Equal("POST", attributes["data-ajax-method"]);
            Assert.Equal("#loadingElement", attributes["data-ajax-loading"]);
            Assert.Equal(450, attributes["data-ajax-loading-duration"]);
            Assert.Equal("http://someurl.com", attributes["data-ajax-url"]);
            Assert.Equal("#someId", attributes["data-ajax-update"]);
            Assert.Equal("before", attributes["data-ajax-mode"]);
            Assert.Equal("some_begin_function", attributes["data-ajax-begin"]);
            Assert.Equal("some_complete_function", attributes["data-ajax-complete"]);
            Assert.Equal("some_failure_function", attributes["data-ajax-failure"]);
            Assert.Equal("some_success_function", attributes["data-ajax-success"]);
        }

        [Fact]
        public void ToUnobtrusiveHtmlAttributesWithOnlyUpdateTargetId()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions { UpdateTargetId = "someId" };

            // Act
            var attributes = options.ToUnobtrusiveHtmlAttributes();

            // Assert
            Assert.Equal(3, attributes.Count);
            Assert.Equal("true", attributes["data-ajax"]);
            Assert.Equal("#someId", attributes["data-ajax-update"]);
            Assert.Equal("replace", attributes["data-ajax-mode"]); // Only added when UpdateTargetId is set
        }

        [Fact]
        public void ToUnobtrusiveHtmlAttributesWithUpdateTargetIdAndExplicitInsertionMode()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions
            {
                InsertionMode = InsertionMode.InsertAfter,
                UpdateTargetId = "someId"
            };

            // Act
            var attributes = options.ToUnobtrusiveHtmlAttributes();

            // Assert
            Assert.Equal(3, attributes.Count);
            Assert.Equal("true", attributes["data-ajax"]);
            Assert.Equal("#someId", attributes["data-ajax-update"]);
            Assert.Equal("after", attributes["data-ajax-mode"]);
        }

        [Fact]
        public void UpdateTargetIdProperty()
        {
            // Arrange
            AjaxOptions options = new AjaxOptions();

            // Act & Assert
            MemberHelper.TestStringProperty(options, "UpdateTargetId", String.Empty);
        }
    }
}
