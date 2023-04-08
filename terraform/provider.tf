terraform {
  backend "s3" {
    bucket               = "febio-tf-state"
    key                  = "febiostudio.tfstate"
    region               = "us-east-1"
    workspace_key_prefix = "febio"
    profile              = "febio"
    encrypt              = true
  }
}
