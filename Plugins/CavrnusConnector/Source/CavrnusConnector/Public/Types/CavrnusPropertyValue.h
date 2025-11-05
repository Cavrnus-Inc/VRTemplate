// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <MathUtil.h>
#include <Math/Color.h>

namespace Cavrnus
{
	struct FPropertyValue
	{
		public:
			enum class PropertyType : uint8
			{
				String = 0,
				Bool = 1,
				Float = 2,
				Color = 3,
				Vector = 4,
				Transform = 5,

				Unset = 100,
			};

			PropertyType PropType = PropertyType::Unset;
			int Priority = 0;

			//Will have one of the following
			FString StringValue = "";
			bool BoolValue = false;
			float FloatValue = 0;
			FLinearColor ColorValue = FLinearColor();
			FVector4 VectorValue = FVector4();
			FTransform TransformValue = FTransform();

			bool operator==(const FPropertyValue& other) const
			{
				if (PropType != other.PropType)
					return false;
				if (Priority != other.Priority)
					return false;

				if (PropType == PropertyType::String)
					return StringValue == other.StringValue;
				if (PropType == PropertyType::Bool)
					return BoolValue == other.BoolValue;
				if (PropType == PropertyType::Float)
					return FMath::IsNearlyEqual(FloatValue, other.FloatValue, .0001f);
				if (PropType == PropertyType::Color)
					return ColorValue.Equals(other.ColorValue, 0.0001);
				if (PropType == PropertyType::Vector)
					return VectorValue.Equals(other.VectorValue, 0.0001f);
				if (PropType == PropertyType::Transform)
				{
					return TransformValue.GetLocation().Equals(other.TransformValue.GetLocation(), 0.001f) &&
						TransformValue.GetRotation().AngularDistance(other.TransformValue.GetRotation()) < 0.00174533f &&
						TransformValue.GetScale3D().Equals(other.TransformValue.GetScale3D(), 0.001f);
				}

				return false;
			}

			FString ToString() const
			{
				FString res = "";
				if (PropType == PropertyType::String)
					res += "String: " + StringValue;
				if (PropType == PropertyType::Bool && BoolValue)
					res += "Bool: true";
				if (PropType == PropertyType::Bool && !BoolValue)
					res += "Bool: false";
				if (PropType == PropertyType::Float)
					res += "Float: " + FString::SanitizeFloat(FloatValue);
				if (PropType == PropertyType::Color)
					res += "Color: " + ColorValue.ToString();
				if (PropType == PropertyType::Vector)
					res += "Vector: " + VectorValue.ToString();
				if (PropType == PropertyType::Transform)
					res += "Transform: " + TransformValue.ToString();
				if (PropType == PropertyType::Unset)
					res += "Unset: No Data";

				return res;
			}

			bool MatchesType(const FPropertyValue& other) const 
			{
				return PropType == other.PropType;
			}

			FPropertyValue() : PropType(PropertyType::Unset), Priority(0) {}

			//IMPORTANT: Can't use constructors because C++ confuses the types (strings read as bools)
			static FPropertyValue StringPropValue(const FString& StringValue, int Priority = 0) 
			{
				FPropertyValue val;
				val.PropType = PropertyType::String;
				val.Priority = Priority;
				val.StringValue = StringValue;

				return val;
			}
			static FPropertyValue BoolPropValue(const bool BoolValue, int Priority = 0)
			{
				FPropertyValue val;
				val.PropType = PropertyType::Bool;
				val.Priority = Priority;
				val.BoolValue = BoolValue;

				return val;
			}
			static FPropertyValue FloatPropValue(const float FloatValue, int Priority = 0)
			{
				FPropertyValue val;
				val.PropType = PropertyType::Float;
				val.Priority = Priority;
				val.FloatValue = FloatValue;

				return val;
			}
			static FPropertyValue ColorPropValue(const FLinearColor& ColorValue, int Priority = 0)
			{
				FPropertyValue val;
				val.PropType = PropertyType::Color;
				val.Priority = Priority;
				val.ColorValue = ColorValue;

				return val;
			}
			static FPropertyValue VectorPropValue(const FVector4& VectorValue, int Priority = 0)
			{
				FPropertyValue val;
				val.PropType = PropertyType::Vector;
				val.Priority = Priority;
				val.VectorValue = VectorValue;

				return val;
			}
			static FPropertyValue TransformPropValue(const FTransform& TransformValue, int Priority = 0)
			{
				FPropertyValue val;
				val.PropType = PropertyType::Transform;
				val.Priority = Priority;
				val.TransformValue = TransformValue;

				return val;
			}
	};
} // namespace Cavrnus
