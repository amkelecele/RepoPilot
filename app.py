from fastapi import FastAPI
from dotenv import load_dotenv
import requests
import json
import os

load_dotenv()

app = FastAPI()

API_KEY = os.getenv("ASI_ONE_KEY")


@app.get("/")
def root():
    return {
        "status": "running"
    }


@app.post("/generate")
async def generate(data: dict):

    prompt = data["prompt"]

    instruction = f"""
You are RepoPilot.

Generate a complete software project.

Rules:

1. Return ONLY JSON.
2. Include:
   - project
   - files
   - contents
3. contents must contain FULL source code.
4. Every file in files must exist in contents.
5. No markdown.
6. No triple backticks.

Format:

{{
"project":"ProjectName",

"files":[
"README.md",
"src/main.cpp"
],

"contents":{{

"README.md":
"# Title",

"src/main.cpp":
"#include <iostream>\\nint main(){{}}"

}}

}}

User request:

{prompt}
"""

    response = requests.post(
        "https://api.asi1.ai/v1/chat/completions",

        headers={
            "Authorization":
            f"Bearer {API_KEY}",

            "Content-Type":
            "application/json"
        },

        json={

            "model":
            "asi1",

            "messages":[
                {
                    "role":
                    "user",

                    "content":
                    instruction
                }
            ]
        }
    )

    raw = response.json()
    try:

        content = (
            raw["choices"][0]
            ["message"]
            ["content"]
        )

        content = (
            content
            .replace(
                "```json",
                ""
            )
            .replace(
                "```",
                ""
            )
            .strip()
        )

        result = json.loads(content)

        return {

        "project":
        result["project"],

        "files":
        result["files"],

        "contents":
        result["contents"]

        }

    except Exception as e:

        print("\nPARSE ERROR:\n")
        print(e)

        print("\nCONTENT:\n")
        print(content)

        return {
            "project":"Error",
            "files":["README.md"]
        }

   
